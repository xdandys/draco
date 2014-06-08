/*
 * swtimer.c
 *
 *  Created on: 4.2.2012
 *      Author: dan
 */


#include "swtimer.h"
#include "swtimer_config.h"
#include <string.h>

struct swTimer {
    uint32_t lastTrigTime;
    uint8_t shotsRemain;
    uint8_t state;
    struct swTimerConfig config;
};

static struct swTimer swTimers[SWTIMER_SLOTS_COUNT];


static uint32_t timeDiff(uint32_t beginMs, uint32_t endMs)
{
    return (beginMs <= endMs) ? (endMs - beginMs) : ((0xffffffff - beginMs) + endMs);
}


static uint8_t validIndex(int8_t index) {
    if ((index >= 0) && (index < SWTIMER_SLOTS_COUNT)) return 1; else return 0;
}

static int8_t getFreeSlot(void)
{
    uint8_t i = 0;
    for (i=0; i < SWTIMER_SLOTS_COUNT; i++) {
        if (swTimers[i].config.timeoutRoutine == 0)
            return (int8_t) i;
    }

    return -1;
}


int8_t swTimerInstall(void(*timeoutRoutine)(void*), void *arg)
{
    int8_t slot = getFreeSlot();
    if (slot < 0) {
        return slot;
    }

    swTimers[slot].state = SWTIMER_STATE_STOPPED;
    swTimers[slot].config.arg = arg;
    swTimers[slot].config.timeoutRoutine = timeoutRoutine;

    return slot;
}

void swTimerUnInstall(int8_t index)
{
    if (validIndex(index))
    swTimers[index].state = SWTIMER_STATE_STOPPED;
    swTimers[index].config.timeoutRoutine = 0;
}


int8_t swTimerStart(int8_t index, uint32_t interval, uint8_t shotsNr)
{
    if (!validIndex(index)) {        
        return -1;
    }

    swTimers[index].config.shotsNr = shotsNr;
    swTimers[index].config.interval = interval;

    swTimers[index].shotsRemain = swTimers[index].config.shotsNr;
    swTimers[index].lastTrigTime = swtGetElapsedMs();

    swTimers[index].state = SWTIMER_STATE_RUNNING;

    return 0;
}

int8_t swTimerStop(int8_t index)
{
    if (!validIndex(index)) {
        return -1;
    }

    swTimers[index].state = SWTIMER_STATE_STOPPED;

    return 0;
}

void swTimerProcess(void)
{
    int8_t i;
    for(i = 0; i < SWTIMER_SLOTS_COUNT; i++)
    {
        if (swTimers[i].state == SWTIMER_STATE_RUNNING) {
            // running timer found
            uint32_t tDiff = timeDiff(swTimers[i].lastTrigTime, swtGetElapsedMs());
            if (tDiff >= swTimers[i].config.interval) {
                // timer timeouted
                swTimers[i].lastTrigTime = swtGetElapsedMs();
                swTimers[i].config.timeoutRoutine(swTimers[i].config.arg);
                if (swTimers[i].shotsRemain != 0) {
                    if (--swTimers[i].shotsRemain == 0) {
                        swTimerStop(i);
                    }
                }
            }
        }
    }
}

uint32_t swTimerTimeToTrig(int8_t index)
{
    if (!(validIndex(index)))
        return 0;

    if (swTimers[index].state == SWTIMER_STATE_STOPPED)
        return 0;

    uint32_t tDiff = timeDiff(swTimers[index].lastTrigTime, swtGetElapsedMs());

    return (swTimers[index].config.interval > tDiff) ? (swTimers[index].config.interval - tDiff) : 0;
}


int8_t swTimerState(int8_t index)
{
    if (!validIndex(index))
        return -1;

    return (int8_t)swTimers[index].state;
}
