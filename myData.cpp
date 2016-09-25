#include "myData.h"

unsigned long startTimeStamp;

bool IS_DEBUG_PRESSED(context * self)
{
    if (digitalRead(DEBUG_BUTTON))
        return false;
    mydelay(self, DEBOUNCE_DELAY);
    if (digitalRead(DEBUG_BUTTON))
        return false;
    mydelay(self, DEBOUNCE_DELAY);
    if (digitalRead(DEBUG_BUTTON))
        return false;
    return true;
}
// Acquire a data record.
void acquireData(context * ctx, data_t * data) {

    data->time = micros() - startTimeStamp;

    data->accelX = ctx->t->readX();
    data->accelY = ctx->t->readY();
    data->accelZ = ctx->t->readZ();

    //t->updateGPS();

    if(ctx->t->gpsFix())
    {
        data->lat = ctx->t->readLat();
        data->lon = ctx->t->readLon();
        data->vel = ctx->t->readVel();
    }
    else
    {
        data->lat = -1;
        data->lon = -1;
        data->vel = -1;
    }
}

void flashDebugLed(context * self, unsigned int ntimes, unsigned int period)
{
    for(int i = 0; i < ntimes; i++)
    {
        digitalWrite(DEBUG_LED,HIGH);
        mydelay(self, period/2);
        digitalWrite(DEBUG_LED,LOW);
        mydelay(self, period/2);
    }
}


#define GPS_SAMP 500 //microseconds
//period in millis
void mydelay(context * self, unsigned int period)
{
    unsigned int iterNum = (period * 1000)/GPS_SAMP;
    for(int i = 0; i < iterNum; i++)
    {
        self->t->updateGPS();
        delayMicroseconds(GPS_SAMP);
    }
}
// Print a data record.
void printData(Print* pr, data_t* data) {
    //pr->printf("%lu %u %.6f %.6f\n\r",data->time, data->accel, data->lat, data->lon);

  pr->print(data->time);

    pr->write(' ');
    pr->print(data->accelX);
    pr->write(' ');
    pr->print(data->accelY);
    pr->write(' ');
    pr->print(data->accelZ);

    pr->write(' ');
  pr->printf("%.6f",data->lat);
    pr->write(' ');
  pr->printf("%.6f",data->lon);
      pr->write(' ');
  pr->printf("%.2f",data->vel);
  pr->println();


}

/*******************************************************************************
  NAME
    averagePower - Average power calculation
  DESCRIPTION
    Calculates average power of given array
*******************************************************************************/
void averagePower(context * ctx, unsigned int *array, size_t length)
{
  unsigned int accu = 0;
  unsigned int    i;

  for (i = 0; i < length; i++)
    accu += array[i] * array[i];

  return accu/length;
}
