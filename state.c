#define  SLEEP_ST 0
#define ALERT1_ST 1
#define ALERT2_ST 2
#define  SETUP_ST 3
#define RECORD_ST 4
#define   SHUT_ST 5

/*******************************************************************************
  SLEEP STATE DEFINES
*******************************************************************************/
#define SLPTIM_SLEEPST 30                  /* sleep time for the sleep state */
#define ACTPER_SLEEPST 1000     /* accel sampling period for the sleep state */
#define ACTTHD_SLEEPST 10000    /* accel power threshold for the sleep state */
#define CNTTHD_SLEEPST 10   /* number of activity events for the sleep state */

/*******************************************************************************
  ALERT1 STATE DEFINES
*******************************************************************************/
#define SLPTIM_ALER1ST 30                  /* sleep time for the alert1 state */
#define ACTPER_ALER1ST 1000     /* accel sampling period for the alert1 state */
#define ACTTHD_ALER1ST 10000    /* accel power threshold for the alert1 state */
#define CNTTHD_ALER1ST 10   /* number of activity events for the alert1 state */

/*******************************************************************************
  ALERT2 STATE DEFINES
*******************************************************************************/
#define SLPTIM_ALER2ST 30                  /* sleep time for the alert2 state */
#define ACTPER_ALER2ST 1000     /* accel sampling period for the alert2 state */
#define ACTTHD_ALER2ST 10000    /* accel power threshold for the alert2 state */
#define CNTTHD_ALER2ST 10   /* number of activity events for the alert2 state */

typedef state unsigned char;

struct context_st
{
  state thisState;
  state nxtState;
  unsigned int actCnt;
  AssetTracker * t;
};

typedef context_st context;

/*******************************************************************************
  NAME
    sleepSt - sleep state
  DESCRIPTION
    Function for the sleep state in the main state machine
*******************************************************************************/
void sleepSt(context * ctx)
{
  ctx->nxtState = ctx->thisState;

  if (accAct(ACTPER_SLEEPST, ACTTHD_SLEEPST))
  {
    ctx->actCnt++;
    if (ctx->actCnt >= CNTTHD_SLEEPST)
    {
      ctx->nxtState = ALET1_ST;
      return;
    }
  }

  gotoSleep(SLPTIM_SLEEPST);
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

  if (accAct(ACTPER_ALER1ST, ACTTHD_ALER1ST))
  {
    ctx->actCnt++;
    if (ctx->actCnt >= CNTTHD_ALER1ST)
    {
      ctx->nxtState = ALET2_ST;
      return;
    }
  }

  gotoSleep(SLPTIM_ALER1ST);
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

  if (gpsAct(ACTTHD_ALER2ST))
  {
    ctx->actCnt++;
    if (ctx->actCnt >= CNTTHD_ALER2ST)
    {
      ctx->nxtState = SETUP_ST;
      return;
    }
  }

  gotoSleep(SLPTIM_ALER2ST);
}

/*******************************************************************************
  NAME
    recordSt - data recording state machine function
  DESCRIPTION
    Actual recording state function.
*******************************************************************************/
void recordSt(context * ctx)
{
  return; // NOOP for the time being TODO: do something funny here
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
    case(REC_ST):
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
