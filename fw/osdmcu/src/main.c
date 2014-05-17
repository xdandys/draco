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

int main(void)
{
    initBoard();
    initSystick();
    DebugSendString("ahoj2");
    osdMain();
    while(1);
    return 0;
}
