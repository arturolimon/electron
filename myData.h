#ifndef myData_h
#define myData_h
#include "myAssetTracker.h"
#include "SdFat.h"

#define GPS_RATE (unsigned long) 100
#define GPS_FIX_ATTEMPTS    60
#define GPS_BACKOFF         1000
#define ADC_DIM             1
#define DEBUG_BUTTON        C0
#define DEBUG_LED           D7
#define DEBOUNCE_DELAY      5
#define LAT_OFFSET          (float) 2000
#define LON_OFFSET          (float) 10300

extern unsigned long startTimeStamp;

typedef unsigned char state;

struct context_st
{
  state         thisState;
  state         nxtState;
  unsigned int  actCnt;
  AssetTracker *t;
  FuelGauge    *fuel;
  SdFat        *sd;
};

typedef struct context_st context;

struct data_st
{
  unsigned long time;
  int accelX;
  int accelY;
  int accelZ;
  float lat;
  float lon;
  float vel;

};

typedef struct data_st data_t;
bool IS_DEBUG_PRESSED(context * self);
void acquireData(context * ctx, data_t * data);
void printData(Print* pr, data_t* data);
void flashDebugLed(context * self, unsigned int ntimes, unsigned int period);
void mydelay(context * self, unsigned int period);

/*******************************************************************************
  NAME
    averagePower - Average power calculation
  DESCRIPTION
    Calculates average power of given array
*******************************************************************************/
unsigned int averagePower(context * ctx, unsigned int *array, size_t length);

#endif
