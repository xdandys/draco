Draco
==========

Draco is opensource HW&SW UAV flight controller board. It was primary designed to my needs, but anybody can manufacture it, use it or modify it. If you would like to own one, you can contact me. 

It has two MCUs on it. The smaller one is used for OSD. The bigger one is running [TauLabs][1] flight code.

![Draco board](/graphics/draco_side_by_side.png?raw=true "Draco board")

###Features###
* main MCU STM32F4, MCU for OSD STM32F303CC
* 32MBit serial flash
* MPU9250 in SPI mode
* MS5611 in SPI mode
* CAN bus transceiver
* microSD slot via SDIO
* 4x A/D inputs
* slot for telemetry module (xbee or PirateBee - see below)
* half-duplex inverted serial line for new FrSky S.PORT telemetry bus
* GPS port with power, UART and I2C for external compass
* 2x extension port with UART/I2C and GPIOs
* extension SPI port
* dedicated sonar port with power and trigger+echo signals
* microUSB
* 12x PWM in/out + 1x S.BUS/CPPM pin on 2.54mm header
* 6x LEDs
* micro button
* OSD with resolution 320x240, automatic PAL/NTSC detection
* dimmensions: 62x47mm

###TauLabs###
Draco is currently one of the non-official TauLabs target. You can find ported code at [my fork][2]. There are still some things missing and I will probably add them in short time:

* support for microSD card logging
* support of FrSky S.PORT telemetry protocol
* micro button support

  [1]: http://taulabs.org/
  [2]: https://github.com/strnadda/TauLabs/tree/draco
