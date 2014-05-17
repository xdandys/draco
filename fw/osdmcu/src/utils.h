/*
 * utils.h
 *
 *  Created on: 15.12.2011
 *      Author: strnad
 */

#ifndef UTILS_H_
#define UTILS_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char *itoa(int val, char *buf, int radix);
extern char *ltoa(long val, char *buf, int radix);
extern char *ultoa(unsigned long val, char *buf, int radix);
extern uint8_t cmstr(char* s1, char* s2, uint8_t n);
extern uint32_t atou(char *s);
extern uint16_t slen(char* s);
extern int lsprintf(char *out, const char *format, ...);
extern uint32_t elapsedMs(void);
#ifdef __cplusplus
}
#endif
#endif /* UTILS_H_ */
