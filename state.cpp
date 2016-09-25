#include "state.h"
#include "myData.h"
#include "log.h"

/*******************************************************************************
  NAME
    gotoSleep- go to sleep
  DESCRIPTION
    Does the appropriate setup before sending electron to sleep.
    Shutting down whatever is needed to reduce power consumption.
  NOTES
    Period is in seconds
*******************************************************************************/
void gotoSleep(context * ctx, unsigned int period)
{
    // TODO: Figure out which variables to move to the backup SRAM
    // System.sleep(SLEEP_MODE_DEEP, period);
}
/*******************************************************************************
  NAME
    accAct - accelerometer activity check
  DESCRIPTION
    This function checks for accelerometer activity and updates the appropriate
    counters.
  NOTES
    Power spectral density will be calculated and a threshold-based approach
    is used to determine if there is activity or not.
  RETURNS
    True when it is safe to say that there is accelerometer activity
*******************************************************************************/
bool accAct(context * ctx, unsigned int period, unsigned int duration,
            unsigned int threshold)
{
  unsigned int i;
  unsigned int nTimes = duration / period;
  unsigned int * array = (unsigned int *)malloc(sizeof(unsigned int) * nTimes);
  unsigned int average = 0;

  for (i = 0; i < nTimes; i++)
    array[i] = ctx->t->readXYZmagnitude();

  // At this point in time we have all we need to calculate power spectral dens

  average = averagePower(ctx, array, nTimes);

  // Cleanup
  free(array);

  return (average >= threshold);
}

/*******************************************************************************
  NAME
    gpsAct - GPS activity check
  DESCRIPTION
    This function checks for GPS activity and updates the appropriate
    counters.
  NOTES
    Since it is not a good idea to use the GPS coordinates to determine if
    there is activity or not, GPS speed will be used instead. A threshold is set
    above potential noise and whatever goes above this limit is defined as
    movement and therefore GPS activity.
  RETURNS
    True when it is safe to say that there is GPS activity.
*******************************************************************************/
bool gpsAct(context * ctx, unsigned int threshold)
{
  return TRUE;
}

/*******************************************************************************
  NAME
    sleepSt - sleep state
  DESCRIPTION
    Function for the sleep state in the main state machine
*******************************************************************************/
void sleepSt(context * ctx)
{
  ctx->nxtState = ctx->thisState;

  if (accAct(ctx, ACTPER_SLEEPST, ACTTHD_SLEEPST))
  {
    ctx->actCnt++;
    if (ctx->actCnt >= CNTTHD_SLEEPST)
    {
      ctx->nxtState = ALERT1_ST;
      return;
    }
  }

  gotoSleep(ctx, SLPTIM_SLEEPST);
}

/*******************************************************************************
  NAME
    alert1St - alert level 1 state
  DESCRIPTION
    Function for the alert level 1 state in the main state machine. This state
    deals only with accelerometer activiy.
*******************************************************************************/
void alert1St(context * ctx)
{
  ctx->nxtState = ctx->thisState;

  if (accAct(ctx, ACTPER_ALER1ST, ACTTHD_ALER1ST))
  {
    ctx->actCnt++;
    if (ctx->actCnt >= CNTTHD_ALER1ST)
    {
      ctx->nxtState = ALERT2_ST;
      return;
    }
  }

  gotoSleep(ctx, SLPTIM_ALER1ST);
}

/*******************************************************************************
  NAME
    aler2St - alert level 2 state
  DESCRIPTION
    Function for the alert level 2 state in the main state machine. This state
    deals with GPS activity.
*******************************************************************************/
void alert2St(context * ctx)
{
  ctx->nxtState = ctx->thisState;

  if (gpsAct(ctx, ACTTHD_ALER2ST))
  {
    ctx->actCnt++;
    if (ctx->actCnt >= CNTTHD_ALER2ST)
    {
      ctx->nxtState = SETUP_ST;
      return;
    }
  }

  gotoSleep(ctx, SLPTIM_ALER2ST);
}

/*******************************************************************************
  NAME
    setupSt - setup state
  DESCRIPTION
    Function for the setup state in the main state machine
  NOTES
    Preparation stage for the recording state. Whatever needs to be done
    before recording starts should be done here.
*******************************************************************************/
void setupSt(context * ctx)
{
}

/*******************************************************************************
  NAME
    recordSt - data recording state machine function
  DESCRIPTION
    Actual recording state function.
*******************************************************************************/
void recordSt(context * ctx)
{
  logData(ctx);
}

/*******************************************************************************
  NAME
    shutSt - shutdown state machine function
  DESCRIPTION
    Shut everything down.
*******************************************************************************/
void shutSt(context * ctx)
{
  return; // NOOP for the time being TODO: do something funny here
}

/*******************************************************************************
  NAME
    advanceState - advance state machine
  DESCRIPTION
    Advance state machine. Prepare all context variables as wells
    power profile.
*******************************************************************************/
void advanceState(context * ctx)
{
  updatePowerProfile(ctx);
  resetContext(ctx);
  ctx->thisState = ctx->nxtState;
}

/*******************************************************************************
  NAME
    resetContext - reset context
  DESCRIPTION
    Prepare global  context for the next state. Reset variables to null.
*******************************************************************************/
void resetContext(context * ctx)
{
  ctx->actCnt = 0;
}

/*******************************************************************************
  NAME
    updatePowerProfile - Update power profile
  DESCRIPTION
    This function turns ON/OFF modules depending on next state.
*******************************************************************************/
void updatePowerProfile(context * ctx)
{
  switch(ctx->nxtState)
  {
    case(SLEEP_ST):
      ctx->t->accOn();
      ctx->t->gpsOff();
      break;
    case(ALERT1_ST):
      //ctx->t->accOn();
      ctx->t->gpsOff();
      break;
    case(ALERT2_ST):
      //ctx->t->accOn();
      ctx->t->gpsOn();
      break;
    case(SETUP_ST):
      //ctx->t->accOn();
      ctx->t->gpsOn();
      break;
    case(RECORD_ST):
      //ctx->t->accOn();
      ctx->t->gpsOn();
      break;
    case(SHUT_ST):
      //ctx->t->accOn();
      ctx->t->gpsOff();
      break;
    default:
      /*TODO: print some sort of error message if we end up here */
      break;
  }
}

/*******************************************************************************
  NAME
    runState - run state machine state
  DESCRIPTION
    Launch the corresponding function depending on current state.
*******************************************************************************/
void runState(context * ctx)
{
  switch(ctx->thisState)
  {
    case(SLEEP_ST):
      sleepSt(ctx);
      break;
    case(ALERT1_ST):
      alert1St(ctx);
      break;
    case(ALERT2_ST):
      alert2St(ctx);
      break;
    case(SETUP_ST):
      setupSt(ctx);
      break;
    case(RECORD_ST):
      recordSt(ctx);
      break;
    case(SHUT_ST):
      shutSt(ctx);
      break;
    default:
      /*TODO: print some sort of error message if we end up here */
      break;
  }
}
