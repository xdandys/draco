/*
 * utils.c
 *
 *  Created on: 15.12.2011
 *      Author: strnad
 */


#include "utils.h"
#include <stdarg.h>

#ifdef __unix__
#include <sys/time.h>
#else
#include "delay.h"
#endif

#define PRINT_BUF_LEN 128
static char print_buf[PRINT_BUF_LEN];

#ifdef __unix__
static int first = 1;
struct timeval startTime;
uint32_t elapsedMs(void)
{
    if (first) {
        gettimeofday(&startTime, 0);
        first = 0;
    }
    struct timeval t2;
    gettimeofday(&t2, 0);

    double elapsedTime;
    elapsedTime = (t2.tv_sec - startTime.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (t2.tv_usec - startTime.tv_usec) / 1000.0;   // us to ms

    return (uint32_t)elapsedTime;
}
#else
uint32_t elapsedMs(void)
{
    return getElapsedMs();
}

#endif

static void xtoa(unsigned long val, char *buf, unsigned radix, int negative)
{
    char *p;
    char *firstdig;
    char temp;
    unsigned digval;

    p = buf;

    if (negative)
    {
        // Negative, so output '-' and negate
        *p++ = '-';
        val = (unsigned long)(-(long) val);
    }

    // Save pointer to first digit
    firstdig = p;

    do
    {
        digval = (unsigned) (val % radix);
        val /= radix;

        // Convert to ascii and store
        if (digval > 9)
            *p++ = (char) (digval - 10 + 'a');
        else
            *p++ = (char) (digval + '0');
    } while (val > 0);

    // We now have the digit of the number in the buffer, but in reverse
    // order.  Thus we reverse them now.

    *p-- = '\0';
    do
    {
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;
        p--;
        firstdig++;
    } while (firstdig < p);
}

char *itoa(int val, char *buf, int radix)
{
    if (radix == 10 && val < 0)
        xtoa((unsigned long) val, buf, radix, 1);
    else
        xtoa((unsigned long)(unsigned int) val, buf, radix, 0);

    return buf;
}

char *ltoa(long val, char *buf, int radix)
{
    xtoa((unsigned long) val, buf, radix, (radix == 10 && val < 0));
    return buf;
}

char *ultoa(unsigned long val, char *buf, int radix)
{
    xtoa(val, buf, radix, 0);
    return buf;
}



// small string compare
uint8_t cmstr(char* s1, char* s2, uint8_t n)
{
    uint8_t i;
    for (i=0; i <n;i++) {
        if ((s1[i]) != (s2[i])) return 0;
        if ((s1[i] == 0 ) || (s2[i] == 0)) return 1;
    }
    return 1;
}

// small string length
uint16_t slen(char* s)
{
    uint16_t len=0;
    while(s[len]!=0) len++;
    return len;
}

uint32_t atou(char *s)
{
    uint8_t i;
    uint32_t res=0x0;
    if ((s[0]!=0) && (s[0]=='0') && (s[1]=='x')) {
        // hex
        for (i=2;i<255;i++) {
            if (((s[i]>='0')&&(s[i]<='9')) || ((s[i]>='a')&&(s[i]<='f'))) {
                uint8_t digVal = (s[i]>'9')?((s[i]-'a') + 10):(s[i]-'0');
                res<<=4;
                res|=digVal;
            } else {
                return res;
            }
        }
    } else {
        // dec
        for (i=0;i<255;i++) {
            if ((s[i]>='0')&&(s[i]<='9')) {
                uint8_t digVal = s[i]-'0';
                res*=10;
                res+=digVal;
            } else {
                return res;
            }
        }
    }
    return 0;
}


static void printchar(char **str, int c)
{
    if (str) {
        **str = c;
        ++(*str);
    }
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
    int pc = 0, padchar = ' ';

    if (width > 0) {
        int len = 0;
        const char *ptr;
        for (ptr = string; *ptr; ++ptr) ++len;
        if (len >= width) width = 0;
        else width -= len;
        if (pad & PAD_ZERO) padchar = '0';
    }
    if (!(pad & PAD_RIGHT)) {
        for ( ; width > 0; --width) {
            printchar (out, padchar);
            ++pc;
        }
    }
    for ( ; *string ; ++string) {
        printchar (out, *string);
        ++pc;
    }
    for ( ; width > 0; --width) {
        printchar (out, padchar);
        ++pc;
    }

    return pc;
}


static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
    char *s;
    int t, neg = 0, pc = 0;
    unsigned int u = i;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints (out, print_buf, width, pad);
    }

    if (sg && b == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN-1;
    *s = '\0';

    while (u) {
        t = u % b;
        if( t >= 10 )
            t += letbase - '0' - 10;
        *--s = t + '0';
        u /= b;
    }

    if (neg) {
        if( width && (pad & PAD_ZERO) ) {
            printchar (out, '-');
            ++pc;
            --width;
        }
        else {
            *--s = '-';
        }
    }

    return pc + prints (out, s, width, pad);
}


static int printfloat(char **out, float f, int width, uint8_t e)
{
    int pc = 0;
    char *s;
    long l = 0;
    int8_t exp = 0;
    union {
        float f;
        uint32_t i;
    } flint;

    flint.f = f;
    if ((flint.i & 0x7fffffff) == 0) {
        // +-0
        return prints(out, "+-0", 0, 0);
    }

    if ((flint.i | 0x80000000) == 0xff800000) {
        // +-infinity
        return prints(out, "+-Inf", 0, 0);
    }

    if ((flint.i | 0x80000000) > 0xff800000) {
        // QNaN / SNaN
        return prints(out, "QNaN/SNaN", 0, 0);
    }

    if (flint.i & 0x80000000) {
        printchar(out, '-');
        f=-f;
    } else {
        printchar(out, '+');
    }
    pc++;

    if (e) {
        if (f < 1.0f) e = 2;

        while (!((f>=1.0f) && (f<10.0f)) && (++exp < 35)) {
            if (e==2) f*=10.0f; else f/=10.0f;
        }
    }

    s = print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';

    /* left-point side */
    l = f;
    do {
        *--s =(char) ((l % 10) + '0');
        l /= 10;
        //if (p == print_buf)
    } while(l);

    pc += prints(out, s, 0, 0);


    /* right-point side */
    s = print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';
    printchar(out, '.');
    pc++;

    long prec = 1;
    int w;
    for (w = width; w>0; w--) prec*=10;
    l = f;
    l = (f - l) * prec;

    do {
        *--s =(char) ((l % 10) + '0');
        l /= 10;
        //if (p == print_buf)
    } while(--width);

    pc+= prints(out, s, 0, 0);

    if (e) {
        printchar(out, 'e');
        if (e==2) printchar(out, '-'); else printchar(out, '+');
        pc+=2;
        pc+=printi(out, exp, 10, 0, 3, PAD_ZERO, 'a');
    }


    return pc;
}

static int print(char **out, const char *format, va_list args )
{
    int width, pad;
    int pc = 0;
    char scr[2];

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = pad = 0;
            if (*format == '\0') break;
            if (*format == '%') goto out;
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }
            for ( ; *format >= '0' && *format <= '9'; ++format) {
                width *= 10;
                width += *format - '0';
            }
            if( *format == 's' ) {
                char *s = (char *)va_arg( args, char* );
                pc += prints (out, s?s:"(null)", width, pad);
                continue;
            }
            if( *format == 'd' ) {
                pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
                continue;
            }
            if( *format == 'x' ) {
                pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'X' ) {
                pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
                continue;
            }
            if( *format == 'u' ) {
                pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'f' ) {
                pc += printfloat (out, (float)va_arg(args, double ), width, 0);
                continue;
            }
            if( *format == 'e' ) {
                pc += printfloat (out, (float)va_arg( args, double ), width, 1);
                continue;
            }

            if( *format == 'c' ) {
                /* char are converted to int then pushed on the stack */
                scr[0] = (char)va_arg( args, int );
                scr[1] = '\0';
                pc += prints (out, scr, width, pad);
                continue;
            }
        }
        else {
        out:
            printchar (out, *format);
            ++pc;
        }
    }
    if (out) **out = '\0';
    va_end( args );
    return pc;
}

int lsprintf(char *out, const char *format, ...)
{
        va_list args;
        va_start( args, format );
        return print( &out, format, args );
}

