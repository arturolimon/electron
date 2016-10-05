#include "myData.h"
#include "myAssetTracker.h"
#include "state.h"
#include "log.h"

//SYSTEM_MODE(MANUAL);

bool start_recording  = FALSE;



/*******************************************************************************
                                GLOBAL VARIABLES
*******************************************************************************/
// Creating an AssetTracker named 'globalTracker' for us to reference
AssetTracker globalTracker = AssetTracker(&Serial);

// A FuelGauge named 'fuel' for checking on the battery state
FuelGauge fuel;

// Secondary SPI with DMA
// SCK => D4, MISO => D3, MOSI => D2, SS => D1
SdFat sd(1);

retained context thisCtx =
{
  .thisState = IDLE_ST,
  .nxtState = IDLE_ST,
  .actCnt    = 0,
  .t         = (AssetTracker *) 0,
  .fuel      = (FuelGauge *) 0,
  .sd        = (SdFat *) 0
};

//------------------------------------------------------------------------------
void setup(void)
{
  int i = 0;

  /* Initialize context */
  thisCtx.t         = &globalTracker;
  thisCtx.fuel      = &fuel;
  thisCtx.sd        = &sd;
  thisCtx.debug     = &Serial;

  pinMode(DEBUG_BUTTON, INPUT);

  if (ERROR_LED_PIN >= 0)
  {
    pinMode(ERROR_LED_PIN, OUTPUT);
  }

  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial)
  {
      delay(1);
    //SysCall::yield();
  }

  /* moved to setupSt
  // added by me. Should print memory and error out if size is not 512
  validateMemory(&thisCtx);

  // initialize file system.
  if (!thisCtx.sd->begin(SD_CS_PIN, SPI_FULL_SPEED))
  {
    thisCtx.sd->initErrorPrint();
    fatalBlink();
  }
  */

  pinMode(DEBUG_LED,OUTPUT);
  digitalWrite(DEBUG_LED,LOW);

  /* moved to accOn gpsOn etc
  pinMode(D6,OUTPUT);
  digitalWrite(D6,LOW);
  delay(100);
  thisCtx.t->begin();
  thisCtx.t->gpsOn();
  */
  thisCtx.t->gpsPowerWellOn();
  thisCtx.t->accOn();
  thisCtx.t->gpsOn();

  /* Asset tracker stuff */
  // Sets up all the necessary AssetTracker bits

  /* TODO: Need to move this to gpsOn maybe */
  for (i= 0; i < GPS_FIX_ATTEMPTS; i++)
  {
    mydelay(&thisCtx, GPS_BACKOFF);
    if(thisCtx.t->gpsFix())
    {
      break;
    }
  }

  flashDebugLed(&thisCtx, 2, 500);
  digitalWrite(DEBUG_LED,HIGH);
}

//------------------------------------------------------------------------------
void loop(void)
{


  runState(&thisCtx);
  advanceState(&thisCtx);

  // discard any input
  /*
  do
  {
    //delay(1);
    thisCtx.t->updateGPS();
  }
  while (Serial.read() >= 0);

  Serial.println();
  Serial.println(F("type:"));
  Serial.println(F("c - convert file to csv"));
  Serial.println(F("d - dump data to Serial"));
  Serial.println(F("e - overrun error details"));
  //Serial.println(F("r - record data"));
  Serial.println(F("Press button to record data"));

  while(!Serial.available())
  {
    if (IS_DEBUG_PRESSED(&thisCtx))
    {
        start_recording = true;
        break;
    }
    thisCtx.t->updateGPS();
    //SysCall::yield();
  }
  char c = tolower(Serial.read());

  // Discard extra Serial data.
  do
  {
    //delay(1);
    thisCtx.t->updateGPS();
  } while (Serial.read() >= 0);

  if (ERROR_LED_PIN >= 0)
  {
    digitalWrite(ERROR_LED_PIN, LOW);
  }

  if (c == 'c')
  {
    binaryToCsv(&thisCtx);
  }
  else if (c == 'd')
  {
    dumpData();
  }
  else if (c == 'e')
  {
    checkOverrun(&thisCtx);
  }
  else if (start_recording == true)
  {
    start_recording = false;
    digitalWrite(DEBUG_LED,LOW);
    logData(&thisCtx);
  }
  else
  {
    Serial.println(F("Invalid entry"));
  }
  */
}
