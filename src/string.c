#include "common.h"

#include "string.h"

static const char const uppercase_hexchars[] = "0123456789ABCDEF";
static const char const lowercase_hexchars[] = "0123456789abcdef";

// FORWARD
static uint32_t uintToStr_(uint32_t value, char *str, int16_t minsize, int16_t maxsize);
static uint16_t skip_atoi(const char **s);
static uint16_t vsprintf(char *buf, const char *fmt, va_list args);


uint32_t strlen(const char *str)
{
    const char *src;

    src = str;
    while (*src++);

    return (src - str) - 1;
}

uint16_t strnlen(const char *str, uint16_t maxlen)
{
    const char *src;

    for (src = str; maxlen-- && *src != '\0'; ++src)
        /* nothing */;

    return src - str;
}

int16_t strcmp(const char *str1, const char *str2)
{
    const uint8_t *p1 = (const uint8_t*) str1;
    const uint8_t *p2 = (const uint8_t*) str2;
    uint8_t c1, c2;

    do
    {
        c1 = *p1++;
        c2 = *p2++;
    }
    while (c1 && (c1 == c2));

    return c1 - c2;
}

char* strclr(char *str)
{
    str[0] = 0;

    return str;
}

char* strcpy(char *to, const char *from)
{
    const char *src;
    char *dst;

    src = from;
    dst = to;
    while ((*dst++ = *src++));

    return to;
}

char* strncpy(char *to, const char *from, uint16_t len)
{
    const char *src;
    char *dst;
    uint16_t i;

    src = from;
    dst = to;
    i = 0;
    while ((i++ < len) && (*dst++ = *src++));

    // end string by null character
    if (i > len) *dst = 0;

    return to;
}

char* strcat(char *to, const char *from)
{
    const char *src;
    char *dst;

    src = from;
    dst = to;
    while (*dst++);

    --dst;
    while ((*dst++ = *src++));

    return to;
}

char *strreplacechar(char *str, char oldc, char newc)
{
    char *s;

    s =  str;
    while(*s)
    {
        if (*s == oldc)
            *s = newc;
        s++;
    }

    return s;
}

void intToStr(int32_t value, char *str, int16_t minsize)
{
    uint32_t v;
    char *dst = str;

    if (value < 0)
    {
        v = -value;
        *dst++ = '-';
    }
    else v = value;

    uintToStr_(v, dst, minsize, 16);
}

void uintToStr(uint32_t value, char *str, int16_t minsize)
{
    uintToStr_(value, str, minsize, 16);
}

static uint32_t uintToStr_(uint32_t value, char *str, int16_t minsize, int16_t maxsize)
{
    uint32_t res;
    int16_t cnt;
    int16_t left;
    char data[16];
    char *src;
    char *dst;

    src = &data[16];
    res = value;
    left = minsize;

    cnt = 0;
    while (res)
    {
        *--src = '0' + (res % 10);
        cnt++;
        left--;
        res /= 10;
    }
    while (left > 0)
    {
        *--src = '0';
        cnt++;
        left--;
    }

    if (cnt > maxsize) cnt = maxsize;

    dst = str;
    while(cnt--) *dst++ = *src++;
    *dst = 0;

    return strlen(str);
}

void intToHex(uint32_t value, char *str, int16_t minsize)
{
    uint32_t res;
    int16_t cnt;
    int16_t left;
    char data[16];
    char *src;
    char *dst;
    const int16_t maxsize = 16;

    src = &data[16];
    res = value;
    left = minsize;

    cnt = 0;
    while (res)
    {
        uint8_t c;

        c = res & 0xF;

        if (c >= 10) c += ('A' - 10);
        else c += '0';

        *--src = c;
        cnt++;
        left--;
        res >>= 4;
    }
    while (left > 0)
    {
        *--src = '0';
        cnt++;
        left--;
    }

    if (cnt > maxsize) cnt = maxsize;

    dst = str;
    while(cnt--) *dst++ = *src++;
    *dst = 0;
}

static uint16_t skip_atoi(const char **s)
{
    uint16_t i = 0;

    while(isdigit(**s))
        i = (i * 10) + *((*s)++) - '0';

    return i;
}


static uint16_t vsprintf(char *buf, const char *fmt, va_list args)
{
    char tmp_buffer[12];
    int16_t i;
    int16_t len;
    int16_t *ip;
    uint16_t num;
    char *s;
    const char *hexchars;
    char *str;
    int16_t left_align;
    int16_t plus_sign;
    int16_t zero_pad;
    int16_t space_sign;
    int16_t field_width;
    int16_t precision;

    for (str = buf; *fmt; ++fmt)
    {
        if (*fmt != '%')
        {
            *str++ = *fmt;
            continue;
        }

        space_sign = zero_pad = plus_sign = left_align = 0;

        // Process the flags
repeat:
        ++fmt;          // this also skips first '%'

        switch (*fmt)
        {
            case '-':
                left_align = 1;
                goto repeat;

            case '+':
                plus_sign = 1;
                goto repeat;

            case ' ':
                if ( !plus_sign )
                    space_sign = 1;

                goto repeat;

            case '0':
                zero_pad = 1;
                goto repeat;
        }

        // Process field width and precision

        field_width = precision = -1;

        if (isdigit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*')
        {
            ++fmt;
            // it's the next argument
            field_width = va_arg(args, int16_t);

            if (field_width < 0)
            {
                field_width = -field_width;
                left_align = 1;
            }
        }

        if (*fmt == '.')
        {
            ++fmt;

            if (isdigit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*')
            {
                ++fmt;
                // it's the next argument
                precision = va_arg(args, int16_t);
            }

            if (precision < 0)
                precision = 0;
        }

        if ((*fmt == 'h') || (*fmt == 'l') || (*fmt == 'L'))
            ++fmt;

        if (left_align)
            zero_pad = 0;

        switch (*fmt)
        {
            case 'c':
                if (!left_align)
                    while(--field_width > 0)
                        *str++ = ' ';

                *str++ = (unsigned char) va_arg(args, int16_t);

                while(--field_width > 0)
                    *str++ = ' ';

                continue;

            case 's':
                s = va_arg(args, char *);

                if (!s)
                    s = "<NULL>";

                len = strnlen(s, precision);

                if (!left_align)
                    while(len < field_width--)
                        *str++ = ' ';

                for (i = 0; i < len; ++i)
                    *str++ = *s++;

                while(len < field_width--)
                    *str++ = ' ';

                continue;

            case 'p':
                if (field_width == -1)
                {
                    field_width = 2 * sizeof(void *);
                    zero_pad = 1;
                }

                hexchars = uppercase_hexchars;
                goto hexa_conv;

            case 'x':
                hexchars = lowercase_hexchars;
                goto hexa_conv;

            case 'X':
                hexchars = uppercase_hexchars;

hexa_conv:
                s = &tmp_buffer[12];
                *--s = 0;
                num = va_arg(args, uint16_t);

                if (!num)
                    *--s = '0';

                while(num)
                {
                    *--s = hexchars[num & 0xF];
                    num >>= 4;
                }

                num = plus_sign = 0;

                break;

            case 'n':
                ip = va_arg(args, int16_t*);
                *ip = (str - buf);
                continue;

            case 'u':
                s = &tmp_buffer[12];
                *--s = 0;
                num = va_arg(args, uint16_t);

                if (!num)
                    *--s = '0';

                while(num)
                {
                    *--s = (num % 10) + 0x30;
                    num /= 10;
                }

                num = plus_sign = 0;

                break;

            case 'd':
            case 'i':
                s = &tmp_buffer[12];
                *--s = 0;
                i = va_arg(args, int16_t);

                if (!i)
                    *--s = '0';

                if (i < 0)
                {
                    num = 1;

                    while(i)
                    {
                        *--s = 0x30 - (i % 10);
                        i /= 10;
                    }
                }
                else
                {
                    num = 0;

                    while(i)
                    {
                        *--s = (i % 10) + 0x30;
                        i /= 10;
                    }
                }

                break;

            default:
                continue;
        }

        len = strnlen(s, precision);

        if (num)
        {
            *str++ = '-';
            field_width--;
        }
        else if (plus_sign)
        {
            *str++ = '+';
            field_width--;
        }
        else if (space_sign)
        {
            *str++ = ' ';
            field_width--;
        }

        if ( !left_align)
        {
            if (zero_pad)
            {
                while(len < field_width--)
                    *str++ = '0';
            }
            else
            {
                while(len < field_width--)
                    *str++ = ' ';
            }
        }

        for (i = 0; i < len; ++i)
            *str++ = *s++;

        while(len < field_width--)
            *str++ = ' ';
    }

    *str = '\0';

    return str - buf;
}

uint16_t sprintf(char *buffer, const char *fmt, ...)
{
    va_list args;
    uint16_t i;

    va_start(args, fmt);
    i = vsprintf(buffer, fmt, args);
    va_end(args);

    return i;
}
