/*
 * version.h
 *
 *  Created on: 3.2.2012
 *      Author: strnad
 */

#ifndef VERSION_H_
#define VERSION_H_


#define DEVICE_DRACO_OSD




#if defined(DEVICE_DRACO_OSD)
    #define VARIANT "a"
    #define DEVIDV  "DRACO-OSD"
#endif

#define VMAJOR  0
#define VMINOR  01


#define V_STRINGIFY(str)   #str
#define STRINGIFY(s) V_STRINGIFY(s)

#define VERSIONSTRING   DEVIDV " " STRINGIFY(VMAJOR) "." STRINGIFY(VMINOR) VARIANT


extern const char __APP_VERSION[];

#endif /* VERSION_H_ */
