#ifndef STATE_H
#define STATE_H
#include "myData.h"
/*******************************************************************************
  STATE DEFINES
*******************************************************************************/
#define  IDLE_ST 0
#define ALERT1_ST 1
#define ALERT2_ST 2
#define  SETUP_ST 3
#define RECORD_ST 4
#define   SHUT_ST 5

/*******************************************************************************
  IDLE STATE DEFINES
*******************************************************************************/
#define SLPTIM_IDLEST 30                  /* sleep time for the sleep state */
#define ACTPER_IDLEST 1000     /* accel sampling period for the sleep state */
#define ACTTHD_IDLEST 10000    /* accel power threshold for the sleep state */
#define CNTTHD_IDLEST 10   /* number of activity events for the sleep state */

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

/*******************************************************************************
  NAME
    gotoSleep- go to sleep
  DESCRIPTION
    Does the appropriate setup before sending electron to sleep.
    Shutting down whatever is needed to reduce power consumption.
*******************************************************************************/
void gotoSleep(context * ctx, unsigned int period);

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
bool accAct(context * ctx, unsigned int period, unsigned int threshold);

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
bool gpsAct(context * ctx, unsigned int threshold);

/*******************************************************************************
  NAME
    sleepSt - sleep state
  DESCRIPTION
    Function for the sleep state in the main state machine
*******************************************************************************/
void sleepSt(context * ctx);

/*******************************************************************************
  NAME
    alert1St - alert level 1 state
  DESCRIPTION
    Function for the alert level 1 state in the main state machine. This state
    deals only with accelerometer activiy.
*******************************************************************************/
void alert1St(context * ctx);

/*******************************************************************************
  NAME
    aler2St - alert level 2 state
  DESCRIPTION
    Function for the alert level 2 state in the main state machine. This state
    deals with GPS activity.
*******************************************************************************/
void alert2St(context * ctx);

/*******************************************************************************
  NAME
    setupSt - setup state
  DESCRIPTION
    Function for the setup state in the main state machine
  NOTES
    Preparation stage for the recording state. Whatever needs to be done
    before recording starts should be done here.
*******************************************************************************/
void setupSt(context * ctx);

/*******************************************************************************
  NAME
    recordSt - data recording state machine function
  DESCRIPTION
    Actual recording state function.
*******************************************************************************/
void recordSt(context * ctx);

/*******************************************************************************
  NAME
    shutSt - shutdown state machine function
  DESCRIPTION
    Shut everything down.
*******************************************************************************/
void shutSt(context * ctx);

/*******************************************************************************
  NAME
    advanceState - advance state machine
  DESCRIPTION
    Advance state machine. Prepare all context variables as wells
    power profile.
*******************************************************************************/
void advanceState(context * ctx);

/*******************************************************************************
  NAME
    resetContext - reset context
  DESCRIPTION
    Prepare global  context for the next state. Reset variables to null.
*******************************************************************************/
void resetContext(context * ctx);

/*******************************************************************************
  NAME
    updatePowerProfile - Update power profile
  DESCRIPTION
    This function turns ON/OFF modules depending on next state.
*******************************************************************************/
void updatePowerProfile(context * ctx);

/*******************************************************************************
  NAME
    runState - run state machine state
  DESCRIPTION
    Launch the corresponding function depending on current state.
*******************************************************************************/
void runState(context * ctx);

#endif
