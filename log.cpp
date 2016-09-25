#include "log.h"

// TODO: Put this on a place that actually makes sense
// Set useSharedSpi true for use of an SPI sensor.
const bool useSharedSpi = true;

/******************************************************************************
                                PRIVATE DEFINITIONS
******************************************************************************/
//------------------------------------------------------------------------------
// Buffer definitions.
//
// The logger will use SdFat's buffer plus BUFFER_BLOCK_COUNT additional
// buffers.
//
#ifndef RAMEND
// Assume ARM. Use total of nine 512 byte buffers.
#define BUFFER_BLOCK_COUNT ((uint8_t) 8)
//
#elif RAMEND < 0X8FF
#error Too little SRAM
//
#elif RAMEND < 0X10FF
// Use total of two 512 byte buffers.
#define BUFFER_BLOCK_COUNT ((uint8_t) 1)
//
#elif RAMEND < 0X20FF
// Use total of five 512 byte buffers.
#define BUFFER_BLOCK_COUNT ((uint8_t) 4)
//
#else  // RAMEND
// Use total of 13 512 byte buffers.
#define BUFFER_BLOCK_COUNT ((uint8_t) 12)
#endif  // RAMEND

// Temporary log file.  Will be deleted if a reset or power failure occurs.
#define TMP_FILE_NAME "tmp_log.bin"

// Size of file base name.  Must not be larger than six.
#define BASE_NAME_SIZE ((uint8_t)(sizeof(FILE_BASE_NAME) - 1))

//Interval between data records in microseconds.
#define LOG_INTERVAL_USEC ((uint32_t)2000)

//------------------------------------------------------------------------------
// File definitions.
//
// Maximum file size in blocks.
// The program creates a contiguous file with FILE_BLOCK_COUNT 512 byte blocks.
// This file is flash erased using special SD commands.  The file will be
// truncated if logging is stopped early.
#define FILE_BLOCK_COUNT ((uint32_t) 256000)

// log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "data"


// max number of blocks to erase per erase call
#define ERASE_SIZE ((uint32_t) 262144L)

// Number of data records in a block.
#define  DATA_DIM ((uint16_t)((512 - 4)/sizeof(data_t)))

//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
#define  FILL_DIM ((uint16_t)(512 - 4 - DATA_DIM*sizeof(data_t)))

char binName[13] = FILE_BASE_NAME "00.bin";

struct block_t
{
  uint16_t count;
  uint16_t overrun;
  data_t data[DATA_DIM];
  uint8_t fill[FILL_DIM];
};

const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 2;

block_t* emptyQueue[QUEUE_DIM];
uint8_t emptyHead;
uint8_t emptyTail;

block_t* fullQueue[QUEUE_DIM];
uint8_t fullHead;
uint8_t fullTail;

SdBaseFile binFile; //TODO: should this go in the global ctx?

// Print data header.
void printHeader(Print* pr)
{
  //pr->println(F("time,A0,A1,A6,A7"));
  pr->println(F(""));
}

// Advance queue index.
inline uint8_t queueNext(uint8_t ht)
{
  return ht < (QUEUE_DIM - 1) ? ht + 1 : 0;
}

//==============================================================================
void fatalBlink()
{
  while (true)
  {
    if (ERROR_LED_PIN >= 0)
    {
      digitalWrite(ERROR_LED_PIN, HIGH);
      delay(200);
      digitalWrite(ERROR_LED_PIN, LOW);
      delay(200);
    }
  }
}
//------------------------------------------------------------------------------
void error(context * ctx, const char* msg)
{
  ctx->sd->errorPrint(msg);
  fatalBlink();
}
//==============================================================================
// Convert binary file to csv file.
void binaryToCsv(context *ctx)
{
  uint8_t lastPct = 0;
  block_t block;
  uint32_t t0 = millis();
  uint32_t syncCluster = 0;
  SdFile csvFile;
  char csvName[13];
  uint32_t tPct;

  if (!binFile.isOpen())
  {
    Serial.println();
    Serial.println(F("No current binary file"));
    return;
  }

  binFile.rewind();
  // Create a new csvFile.
  strcpy(csvName, binName);
  strcpy(&csvName[BASE_NAME_SIZE + 3], "csv");

  if (!csvFile.open(csvName, O_WRITE | O_CREAT | O_TRUNC))
    error(ctx,"open csvFile failed");

  Serial.println();
  Serial.print(F("Writing: "));
  Serial.print(csvName);
  Serial.println(F(" - type any character to stop"));
  printHeader(&csvFile);

  tPct = millis();
  while (!Serial.available() && binFile.read(&block, 512) == 512)
  {
    uint16_t i;

    if (block.count == 0)
      break;

    if (block.overrun)
    {
      csvFile.print(F("OVERRUN,"));
      csvFile.println(block.overrun);
    }

    for (i = 0; i < block.count; i++)
      printData(&csvFile, &block.data[i]);

    if (csvFile.curCluster() != syncCluster)
    {
      csvFile.sync();
      syncCluster = csvFile.curCluster();
    }

    if ((millis() - tPct) > 1000)
    {
      uint8_t pct = binFile.curPosition()/(binFile.fileSize()/100);
      if (pct != lastPct)
      {
        tPct = millis();
        lastPct = pct;
        Serial.print(pct, DEC);
        Serial.println('%');
      }
    }

    if (Serial.available())
      break;

  }
  csvFile.close();
  Serial.print(F("Done: "));
  Serial.print(0.001*(millis() - t0));
  Serial.println(F(" Seconds"));
}
//------------------------------------------------------------------------------
// read data file and check for overruns
void checkOverrun(context * ctx) {
  bool headerPrinted = false;
  block_t block;
  uint32_t bgnBlock, endBlock;
  uint32_t bn = 0;

  if (!binFile.isOpen())
  {
    Serial.println();
    Serial.println(F("No current binary file"));
    return;
  }

  if (!binFile.contiguousRange(&bgnBlock, &endBlock))
    error(ctx, "contiguousRange failed");

  binFile.rewind();
  Serial.println();
  Serial.println(F("Checking overrun errors - type any character to stop"));
  while (binFile.read(&block, 512) == 512)
  {

    if (block.count == 0)
      break;

    if (block.overrun)
    {
      if (!headerPrinted)
      {
        Serial.println();
        Serial.println(F("Overruns:"));
        Serial.println(F("fileBlockNumber,sdBlockNumber,overrunCount"));
        headerPrinted = true;
      }

      Serial.print(bn);
      Serial.print(',');
      Serial.print(bgnBlock + bn);
      Serial.print(',');
      Serial.println(block.overrun);
    }
    bn++;
  }

  if (!headerPrinted)
    Serial.println(F("No errors found"));
  else
    Serial.println(F("Done"));

}
//------------------------------------------------------------------------------
// dump data file to Serial
void dumpData()
{
  block_t block;
  if (!binFile.isOpen())
  {
    Serial.println();
    Serial.println(F("No current binary file"));
    return;
  }

  binFile.rewind();
  Serial.println();
  Serial.println(F("Type any character to stop"));
  delay(1000);
  printHeader(&Serial);

  while (!Serial.available() && binFile.read(&block , 512) == 512)
  {
    if (block.count == 0)
      break;

    if (block.overrun)
    {
      Serial.print(F("OVERRUN,"));
      Serial.println(block.overrun);
    }

    for (uint16_t i = 0; i < block.count; i++)
      printData(&Serial, &block.data[i]);
  }
  Serial.println(F("Done"));
}

void logData(context * ctx)
{
  bool saveFile = FALSE;
  bool errorSig = FALSE;
  uint32_t bgnBlock, endBlock;

  // Allocate extra buffer space.
  block_t block[BUFFER_BLOCK_COUNT];
  block_t* curBlock = 0;
  uint8_t* cache;

  uint32_t bgnErase;
  uint32_t endErase;

  uint32_t bn           = 0;
  uint32_t t0           = 0;
  uint32_t t1           = 0;
  uint32_t overrun      = 0;
  uint32_t overrunTotal = 0;
  uint32_t count        = 0;
  uint32_t maxDelta     = 0;
  uint32_t minDelta     = 99999;
  uint32_t maxLatency   = 0;

  uint32_t logTime = 0;
  bool closeFile   = false;
  uint32_t delta   = 0;

  // Get address of block to write.
  block_t* pBlock = (block_t *) 0;
  uint32_t usec   = 0;

  Serial.println();

  // Find unused file name.
  if (BASE_NAME_SIZE > 6)
    error(ctx, "FILE_BASE_NAME too long");

  while (ctx->sd->exists(binName))
  {
    if (binName[BASE_NAME_SIZE + 1] != '9')
    {
      binName[BASE_NAME_SIZE + 1]++;
    }
    else
    {
      binName[BASE_NAME_SIZE + 1] = '0';

      if (binName[BASE_NAME_SIZE] == '9')
        error(ctx, "Can't create file name");

      binName[BASE_NAME_SIZE]++;
    }
  }
  // Delete old tmp file.
  if (ctx->sd->exists(TMP_FILE_NAME))
  {
    Serial.println(F("Deleting tmp file"));
    if (!ctx->sd->remove(TMP_FILE_NAME))
      error(ctx, "Can't remove tmp file");
  }
  // Create new file.
  Serial.println(F("Creating new file"));
  binFile.close();

  if (!binFile.createContiguous(ctx->sd->vwd(), TMP_FILE_NAME, 512 * FILE_BLOCK_COUNT))
    error(ctx, "createContiguous failed");

  // Get the address of the file on the SD.
  if (!binFile.contiguousRange(&bgnBlock, &endBlock))
    error(ctx,"contiguousRange failed");

  // Use SdFat's internal buffer.
  cache = (uint8_t*)ctx->sd->vol()->cacheClear();

  if (cache == 0)
    error(ctx, "cacheClear failed");

  // Flash erase all data in the file.
  Serial.println(F("Erasing all data"));
  bgnErase = bgnBlock;

  while (bgnErase < endBlock)
  {
    endErase = bgnErase + ERASE_SIZE;
    if (endErase > endBlock)
    {
      endErase = endBlock;
    }
    if (!ctx->sd->card()->erase(bgnErase, endErase))
    {
      error(ctx,"erase failed");
    }
    bgnErase = endErase + 1;
  }
  // Start a multiple block write.
  if (!ctx->sd->card()->writeStart(bgnBlock, FILE_BLOCK_COUNT))
    error(ctx, "writeBegin failed");

  // Set chip select high if other devices use SPI.
  if (useSharedSpi)
    ctx->sd->card()->chipSelectHigh();

  // Initialize queues.
  emptyHead = emptyTail = 0;
  fullHead = fullTail = 0;

  // Use SdFat buffer for one block.
  emptyQueue[emptyHead] = (block_t*)cache;
  emptyHead = queueNext(emptyHead);

  // Put rest of buffers in the empty queue.
  for (uint8_t i = 0; i < BUFFER_BLOCK_COUNT; i++)
  {
    emptyQueue[emptyHead] = &block[i];
    emptyHead = queueNext(emptyHead);
  }

  flashDebugLed(ctx, 2,500);
  Serial.println(F("Logging - Press button to stop"));

  // Wait for Serial Idle.
  Serial.flush();
  delay(10);

  t0 = millis();
  t1 = t0;

  // Start at a multiple of interval.
  logTime = micros()/LOG_INTERVAL_USEC + 1;
  logTime *= LOG_INTERVAL_USEC;

  startTimeStamp = micros();

  while (1)
  {
    // Time for next data record.
    logTime += LOG_INTERVAL_USEC;

    //if (Serial.available()) {
    if (IS_DEBUG_PRESSED(ctx))
    {
      closeFile = true;
      saveFile  = true;
    }

    if (closeFile)
    {
      if (curBlock != 0)
      {
        // Put buffer in full queue.
        fullQueue[fullHead] = curBlock;
        fullHead = queueNext(fullHead);
        curBlock = 0;
      }
    }
    else
    {
      if (curBlock == 0 && emptyTail != emptyHead)
      {
        curBlock = emptyQueue[emptyTail];
        emptyTail = queueNext(emptyTail);
        curBlock->count = 0;
        curBlock->overrun = overrun;
        overrun = 0;
      }

      do
      {
          delayMicroseconds(1);
          ctx->t->updateGPS();
      }
      while(logTime > micros());
      /*
      int32_t usec = logTime - micros();
      if (usec < 0) {
        error("Rate too fast");
      }
      else
      {
          delayMicroseconds(usec);
      }
      */
      delta = micros() - logTime;

      if (delta > maxDelta)
        maxDelta = delta;
      if (delta < minDelta)
        minDelta = delta;

      if (curBlock == 0)
      {
        overrun++;
      }
      else
      {
        acquireData(ctx, &curBlock->data[curBlock->count++]);

        if (curBlock->count == DATA_DIM)
        {
          fullQueue[fullHead] = curBlock;
          fullHead = queueNext(fullHead);
          curBlock = 0;
        }
      }
    }
    if (fullHead == fullTail)
    {
      // Exit loop if done.
      if (closeFile)
      {
        flashDebugLed(ctx, 2, 500);
        break;
      }
    }
    else if (!ctx->sd->card()->isBusy())
    {
      // Get address of block to write.
      pBlock = fullQueue[fullTail];
      fullTail = queueNext(fullTail);
      // Write block to SD.
      usec = micros();

      if (!ctx->sd->card()->writeData((uint8_t*)pBlock))
        error(ctx, "write data failed");

      usec = micros() - usec;
      t1 = millis();

      if (usec > maxLatency)
        maxLatency = usec;

      count += pBlock->count;

      // Add overruns and possibly light LED.
      if (pBlock->overrun)
      {
        overrunTotal += pBlock->overrun;
        if (ERROR_LED_PIN >= 0)
          digitalWrite(ERROR_LED_PIN, HIGH);
      }
      // Move block to empty queue.
      emptyQueue[emptyHead] = pBlock;
      emptyHead = queueNext(emptyHead);
      bn++;
      if (bn == FILE_BLOCK_COUNT)
        break; // File full so stop
    }
  }
  if (!ctx->sd->card()->writeStop())
  {
    error(ctx, "writeStop failed");
    errorSig = TRUE;
  }
  // Truncate file if recording stopped early.
  if (bn != FILE_BLOCK_COUNT)
  {
    Serial.println(F("Truncating file"));
    if (!binFile.truncate(512L * bn))
    {
      error(ctx, "Can't truncate file");
      errorSig = TRUE;
    }
  }
  if (!binFile.rename(ctx->sd->vwd(), binName))
  {
    error(ctx, "Can't rename file");
     errorSig = TRUE;
  }

  Serial.print(F("File renamed: "));
  Serial.println(binName);
  Serial.print(F("Max block write usec: "));
  Serial.println(maxLatency);
  Serial.print(F("Record time sec: "));
  Serial.println(0.001*(t1 - t0), 3);
  Serial.print(minDelta);
  Serial.print(F(" <= jitter microseconds <= "));
  Serial.println(maxDelta);
  Serial.print(F("Sample count: "));
  Serial.println(count);
  Serial.print(F("Samples/sec: "));
  Serial.println((1000.0)*count/(t1-t0));
  Serial.print(F("Overruns: "));
  Serial.println(overrunTotal);

  if (saveFile && !errorSig)
    binaryToCsv(ctx);

  flashDebugLed(ctx, 3, 500);
  Serial.println(F("Done"));
}

void validateMemory(context * ctx)
{
  Serial.print(F("FreeMemory: "));
  Serial.println(System.freeMemory());
  Serial.print(F("Records/block: "));
  Serial.println(DATA_DIM);

  if (sizeof(block_t) != 512)
  {
    error(ctx, "Invalid block size");
  }
}
