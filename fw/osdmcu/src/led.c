/*
    DRACO - Copyright (C) 2013-2014 Daniel Strnad
                 
    This file is part of DRACO project.

    DRACO is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    DRACO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @file    led.c
 * @brief   brief description here
 *
 */

#include "board.h"
#include "led.h"
#include "swtimer.h"

#define PROCESS_PERIOD          10


typedef struct {
    uint32_t period;
    uint32_t duty;
} LedMode;

typedef struct {
    const LedMode *mode;
    GpioAlias gpio;
} Led;


static int8_t swt;


static Led leds[] = {
        [LED_RED] = {
          .gpio = PinLedRed,
        },
        [LED_GREEN] = {
          .gpio = PinLedGreen,
        },
        [LED_BLUE] = {
          .gpio = PinLedBlue,
        },
        [LED_ORANGE] = {
          .gpio = PinLedOrange,
        },
};

static const LedMode modes[] = {
        [LED_MODE_OFF] = {
          .duty = 0,
        },

        [LED_MODE_ON] = {
          .duty = 100,
        },

        [LED_MODE_BLINK_SLOW] = {
          .duty = 40,
          .period = 800,
        },

        [LED_MODE_BLINK_FAST] = {
          .duty = 50,
          .period = 100,
        },

};

static Led *ledIndexToLed(uint8_t led)
{
    if (led > ((sizeof(leds) / sizeof(Led)) - 1))
        return 0;

    return &leds[led];
}

static const LedMode *modeIndexToMode(uint8_t mode)
{
    if (mode > ((sizeof(modes) / sizeof(LedMode)) - 1))
        return 0;

    return &modes[mode];
}



static void processLed(Led *led, uint32_t ticksMs)
{
    if ((led->mode->duty == 0) || (led->mode->duty == 100))
        return;

    uint32_t phase = ((uint32_t)led->mode->period * 100) / (ticksMs % led->mode->period);
    if (phase > led->mode->duty)
        gpioControl(led->gpio, 0);
    else
        gpioControl(led->gpio, 1);

}

static void setLedMode(Led *led, const LedMode *mode)
{
    led->mode = mode;
    if (mode->duty == 100)
        gpioControl(led->gpio, 1);
    else if (mode->duty == 0)
        gpioControl(led->gpio, 0);
}



static void swtHandler(void *arg)
{
    (void) arg;
    static uint32_t ticksMs = 0;
    ticksMs += PROCESS_PERIOD;

    uint8_t i;
    for(i = 0; i < (sizeof(leds) / sizeof(Led)); i++) {
        Led *led = ledIndexToLed(i);
        processLed(led, ticksMs);
    }
}


void ledControl(uint8_t led, uint8_t mode)
{
    Led *pled = ledIndexToLed(led);
    const LedMode *pmode = modeIndexToMode(mode);

    if ((pled) && (pmode))
        setLedMode(pled, pmode);
}

void ledInit(void)
{
    swt = swTimerInstall(swtHandler, 0);
    swTimerStart(swt, PROCESS_PERIOD, 0);
}
