/*
 * debug.h
 *
 *  Created on: 23.1.2012
 *      Author: strnad
 */

#ifndef DEBUG_H_
#define DEBUG_H_


#define DEBUG_LISTING_ON

#include "delay.h"

#define DEBUG_STR_FORMAT    "[%03u.%03u] [%s:%s] "

void DebugSendString(const char *str);
int DebugPrintf(const char *fmt,...);
void _dhexprint(const char *file, const char *function, const uint8_t *dataPointer, uint16_t len);
void _dbarprint(const char *file, const char *function, uint32_t value, uint32_t rangeMin, uint32_t rangeMax, uint8_t size);

#ifdef DEBUG_LISTING_ON



#define dprint(format, ...) DebugPrintf(DEBUG_STR_FORMAT format "\r\n",  \
        getElapsedMs() / 1000, getElapsedMs() % 1000, __FILE__, __FUNCTION__, ##__VA_ARGS__)


#define dhexprint(data, len) _dhexprint(__FILE__, __FUNCTION__, (data), (len))

#define dbarprint(value, rangeMin, rangeMax, size) _dbarprint(__FILE__, __FUNCTION__, (value), (rangeMin), (rangeMax), (size))

#else
#define dprint(format,...)
#define dhexprint(data,len)
#define dbarprint(value, rangeMin, rangeMax, size)
#endif


void DebugInit(void);
int sprintf(char *out, const char *format, ...);


#endif /* DEBUG_H_ */
