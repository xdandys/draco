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

#define PUTLE16(buff, val) {(buff)[0] = (val & 0xff); (buff)[1] = (val >> 8) & 0xff;}
#define PUTLE32(buff, val) {(buff)[0] = (val & 0xff); (buff)[1] = (val >> 8) & 0xff;(buff)[2] = (val >> 16) & 0xff;(buff)[3] = (val >> 24) & 0xff;}


#define GETLE16(buff) (((uint16_t)(buff)[1] << 8) | ((uint16_t)(buff)[0]))
#define GETLE32(buff) (((uint32_t)(buff)[3] << 24) |((uint32_t)(buff)[2] << 16) | ((uint32_t)(buff)[1] << 8) | ((uint32_t)(buff)[0]))



static inline void PUTLE8_STREAM(uint8_t *buff, uint16_t *offset, uint8_t val)
{
    buff[(*offset)++] = val & 0xff;
}

static inline void PUTLE16_STREAM(uint8_t *buff, uint16_t *offset, uint16_t val)
{
    buff[(*offset)++] = val & 0xff;
    buff[(*offset)++] = (val >> 8) & 0xff;
}

static inline void PUTLE32_STREAM(uint8_t *buff, uint16_t *offset, uint32_t val)
{
    buff[(*offset)++] = val & 0xff;
    buff[(*offset)++] = (val >> 8) & 0xff;
    buff[(*offset)++] = (val >> 16) & 0xff;
    buff[(*offset)++] = (val >> 24) & 0xff;
}


static inline uint8_t GETLE8_STREAM(uint8_t *buff, uint16_t *offset)
{
    uint8_t val = buff[*offset];
    (*offset) += 1;
    return val;
}


static inline uint16_t GETLE16_STREAM(uint8_t *buff, uint16_t *offset)
{
    uint16_t val = (((uint16_t)buff[*offset + 1] << 8) | ((uint16_t)buff[*offset + 0]));
    (*offset) += 2;
    return val;
}


static inline uint32_t GETLE32_STREAM(uint8_t *buff, uint16_t *offset)
{
    uint32_t val = (((uint32_t)buff[*offset + 3] << 24) | ((uint32_t)buff[*offset + 2] << 16) | ((uint32_t)buff[*offset + 1] << 8) | ((uint32_t)buff[*offset + 0]));
    (*offset) += 4;
    return val;
}


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
