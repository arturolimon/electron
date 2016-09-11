#define SLEEP_ST 0
#define ALET1_ST 1

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

struct stctx_st
{
  state thisState;
  state nxtState;
  unsigned int actCnt;
};

typedef stctx stctx_st;

/*******************************************************************************
  NAME
    sleepSt - sleep state
  DESCRIPTION
    Function for the sleep state in the main state machine
*******************************************************************************/
void sleepSt(stctx * ctx)
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
void alert1St(stctx * ctx)
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
void alert2St(stctx * ctx)
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
