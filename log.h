#ifndef LOG_H
#define LOG_H

#include "myAssetTracker.h"
#include "myData.h"

// Digital pin to indicate an error, set to -1 if not used.
// The led blinks for fatal errors. The led goes on solid for SD write
// overrun errors and logging continues.
#define ERROR_LED_PIN ((int8_t) -1)

// Print data header.
void printHeader(Print* pr);
// Advance queue index.
inline uint8_t queueNext(uint8_t ht);
//==============================================================================
void fatalBlink();
//------------------------------------------------------------------------------
void error(context * ctx, const char* msg);
//==============================================================================
// Convert binary file to csv file.
void binaryToCsv(context * ctx);
//------------------------------------------------------------------------------
// read data file and check for overruns
void checkOverrun(context * ctx);
//------------------------------------------------------------------------------
// dump data file to Serial
void dumpData();
//------------------------------------------------------------------------------
// log data

void logData(context * ctx);

void validateMemory(context * ctx);

#endif
