/*
 * swtimer.h
 *
 *  Created on: 4.2.2012
 *      Author: strnad@home
 *
 *   SW timers
 *   only external fuction required is getElapsedMs()
 *   processSwTimer() routine must be called as frequently as possible
 */

#ifndef SWTIMER_H_
#define SWTIMER_H_

#include <stdint.h>


#define SWTIMER_STATE_STOPPED       0
#define SWTIMER_STATE_RUNNING       1

struct swTimerConfig
{
    /**
     * timer interval in miliseconds
     */
    uint32_t interval;
    /**
     * number of timer hits, when reached, timer is stopped, 0 = infinite
     */
    uint8_t shotsNr;
    /**
     * timeout routine
     */
    void (*timeoutRoutine)(void*);
    /**
     * arg that will be pased to timeoutRoutine
     */
    void *arg;
};


/**
 * Install timer
 * @param [in] timeoutRoutine timeout routine
 * @param [in] arg that will be pased to timeoutRoutine
 * @return index of timer slot if succeeded, or negative value when failed
 */

int8_t swTimerInstall(void(*timeoutRoutine)(void*), void *arg);


/**
 * Un-Install timer
 * @param [in] index number of timer (returned by swTimerInstall())
 */


void swTimerUnInstall(int8_t index);

/**
 * Start timer
 * @param [in] index number of timer (returned by swTimerInstall())
 * @param [in] interval timer interval in miliseconds
 * @param [in] shotsNr number of timer hits, 0 = infinite
 * @return negative value when failed
 */

int8_t swTimerStart(int8_t index, uint32_t interval, uint8_t shotsNr);

/**
 * Get time remain to next timer triggering
 * @param [in] index number of timer (returned by swTimerInstall())
 * @return time remain to next timer triggering in miliseconds
 */
uint32_t swTimerTimeToTrig(int8_t index);


/**
 * Get timer state
 * @param [in] index number of timer (returned by swTimerInstall())
 * @return either SWTIMER_STATE_STOPPED or SWTIMER_STATE_RUNNING
 */
int8_t swTimerState(int8_t index);


/**
 * Stop timer
 * @param [in] index number of timer (returned by swTimerInstall())
 * @return negative value when failed
 */

int8_t swTimerStop(int8_t index);

/**
 * Call in main loop as frequently as possible
 * all timer routines are called within context of this function
 */

void swTimerProcess(void);

#endif /* SWTIMER_H_ */
