/*
 * main.c
 *
 *  Created on: 29.10.2012
 *      Author: dan
 */


/*
 * TIM4  .. Motor
 * TIM19 .. Cutter
 * TIM13 .. Burn pulses
 * TIM3  .. Light
 *
 */


#include "debug.h"
#include "delay.h"
#include "board.h"
#include "osdspi.h"
#include "led.h"
#include "delay.h"
#include "osdpainter.h"
#include "osdscreen.h"
#include "utils.h"
#include "comm.h"
#include "hud.h"
#include "swtimer.h"

int main(void)
{
    boardInit();
    sysTimerInit();
    ledInit();
    hudInit(&spiOsdDevice);
    commStart();
    while(1) {
        hudProcess();
        commProcess();
        swTimerProcess();
    }

    return 0;
}
