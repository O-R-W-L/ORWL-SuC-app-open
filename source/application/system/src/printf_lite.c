/*
 * printf_lite.c --
 *
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, Maxim Integrated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MAXIM INTEGRATED ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* [INTERNAL] ------------------------------------------------------------------
 * Created on: Feb 06, 2012
 * Author: Yann G. (yann.gaude@maximintegrated.com)
 *
 * ---- Subversion keywords (need to set the keyword property)
 * $Revision:: $: Revision of last commit
 * $Author:: $: Author of last commit
 * $Date:: $: Date of last commit
 * [/INTERNAL] -------------------------------------------------------------- */

/** Global includes */
#include <printf_lite.h>

#include <stdarg.h> /** For va_start(), etc ... */
#include <config.h>
#include <errors.h>
/** Other includes */
#include <cobra_defines.h>
#include <cobra_macros.h>
#include <cobra_functions.h>
/** Local includes */
#include <private.h>

/* Freertos includes */
#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>

char *gTempPtr ;
unsigned int sizeRxBuff ;
unsigned int  __index = 0;

void write_num(int  n,
				int  base,
				int  sign,
				int  pfzero,
				int  width);
void write_string(const char *text);
int check_string(const char *str);
int private_printf(void (*putc)(char c),
								char *fmt,
								va_list ap);
void write_char(char c);
int write_char_buf(const char c);
void write_long(unsigned long long  n,
					int base,
					int sign,
					int pfzero,
					int width);
void write_hex(int n);
int cvt(unsigned long long val,
					char *buf,
					long radix,
					char *digits);

/******************************************************************************/

int lite_printf(const char *fmt, ...)
{
    int							result = COMMON_ERR_UNKNOWN;
	va_list						ap;

    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
	taskENTER_CRITICAL();
    }

    va_start(ap, fmt);
    result = private_printf(write_char, (char*)fmt, ap);
    va_end(ap);

    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
	taskEXIT_CRITICAL();
    }
    /** We're done */
    return result;
}
/******************************************************************************/

int lite_printf_isr(const char *fmt, ...)
{
    int							result = COMMON_ERR_UNKNOWN;
	va_list						ap;

    va_start(ap, fmt);
    result = private_printf(write_char, (char*)fmt, ap);
    va_end(ap);

    /** We're done */
    return result;
}
/******************************************************************************/

int lite_snprintf(char *str, int size, const char *fmt, ...)
{
    int							result = COMMON_ERR_UNKNOWN;
	va_list						ap;

    // Let us initilize

	sizeRxBuff = size;
	gTempPtr = str ;
	__index = 0 ;

    va_start(ap, fmt);
    result = private_printf(write_char_buf, (char*)fmt, ap);
    va_end(ap);
    /** We're done */
    return result;
}
/******************************************************************************/

void write_char(const char c)
{
	/** Send character */
    while( uart_write_char(c) != NO_ERROR ) ;
    /** Translate LF into CRLF */
    if ( '\n' == c )
    {
        while( uart_write_char('\r') != NO_ERROR ) ;
    }
    /** We're done */
    return;
}
/******************************************************************************/
int write_char_buf(const char c)
{

   if(__index<sizeRxBuff)
   {
       gTempPtr[__index] =  c ;
       __index++ ;
   }
   else
   {
       gTempPtr[__index-1] = '\0' ;
   }
    return NO_ERROR;
}
/******************************************************************************/
/* Write zero terminated string                                         */
void write_string(const char *psz)
{
    while( *psz )
	{
    	write_char(*psz++);
	}
    /** We're done */
    return;
}

/******************************************************************************/
/* Write decimal value                                                  */
void write_dec(int n)
{
    int							sign = '+';

    if ( n < 0 )
    {
    	n = -n;
    	sign = '-';
    }
    /**  */
    write_num( n, 10, sign, FALSE, 0);
    /** We're done */
    return;
}

/******************************************************************************/
/* Write hexadecimal value                                              */
void write_hex(int n)
{
    write_num( n, 16, '+', FALSE, 0);
}

/******************************************************************************/
/* Generic number writing function                                      */
/* The parameters determine what radix is used, the signed-ness of the  */
/* number, its minimum width and whether it is zero or space filled on  */
/* the left.                                                            */

void write_long(unsigned long long  n, /* number to write              */
					int base,           /* radix to write to            */
					int sign,           /* sign, '-' if -ve, '+' if +ve */
					int pfzero,         /* prefix with zero ?           */
					int width)          /* min width of number          */
{
    char					buf[32];
    int						bpos;
    char					bufinit = pfzero? '0':' ';
    char					*digits = "0123456789ABCDEF";

    /* init buffer to padding char: space or zero */
    for( bpos = 0; bpos < (int)sizeof(buf); bpos++ )
    {
    	buf[bpos] = bufinit;
    }
    /* Set pos to start */
    bpos = 0;
    /* Construct digits into buffer in reverse order */
    if( n == 0 )
    {
    	buf[bpos++] = '0';
    }
    else while( n != 0 )
    {
        int d = n % base;
        buf[bpos++] = digits[d];
        n /= base;
    }

    /* set pos to width if less. */
    if( width > bpos ) bpos = width;

    /* set sign if negative. */
    if( sign == '-' )
    {
        if( buf[bpos-1] == bufinit ) bpos--;
        buf[bpos] = sign;
    }
    else bpos--;

    /* Now write it out in correct order. */
    while( bpos >= 0 )
    {
        write_char( buf[bpos--]);
    }
    /**  */
    return;
}

/******************************************************************************/
void write_num(int n, int base, int sign, int pfzero, int width)
{
    write_long((unsigned long long)n, base, sign, pfzero, width);
}


/******************************************************************************/
/* perform some simple sanity checks on a string to ensure that it      */
/* consists of printable characters and is of reasonable length.        */
int check_string(const char *str)
{
    int result = TRUE;
    const char *s;

    if( str == NULL ) return FALSE;

    for( s = str ; result && *s ; s++ )
    {
        char c = *s;

        /* Check for a reasonable length string. */

        if( s-str > 2048 ) result = FALSE;

        /* We only really support CR, NL, and backspace at present. If
         * we want to use tabs or other special chars, this test will
         * have to be expanded.  */

        if( c == '\n' || c == '\r' || c == '\b' || c == '\t' )
            continue;

        /* Check for printable chars. This assumes ASCII */

        if( c < ' ' || c > '~' )
            result = FALSE;

    }

    return result;
}

/******************************************************************************/
int cvt(unsigned long long val,
			char *buf,
			long radix,
			char *digits)
{
    char temp[80];
    char *cp = temp;
    int length = 0;
    unsigned int v = (unsigned int) val; /* to avoid a wrong division with
                                               long long number */
    if ( !v )
    {
        /* Special case */
        *cp++ = '0';
    }
    else
    {
        while (v)
        {
            *cp++ = digits[v % radix];
            v /= radix;
        }
    }
    while (cp != temp)
    {
        *buf++ = *--cp;
        length++;
    }
    *buf = '\0';
    return (length);
}

/******************************************************************************/
int private_printf(void (*putc)(const char c),
								char *fmt,
								va_list ap)
{
    char						buf[sizeof(unsigned long long) * 8];
    char						c;
    char						sign;
    char						*cp = buf;
    int							left_prec;
    int							right_prec;
    int							zero_fill;
    int							pad;
    int							pad_on_right;
    int							i;
    int							islong;
    int							islonglong;
    long long					val = 0;
    int							res = 0;
    int							length = 0;

    if ( FALSE == check_string(fmt) )
    {
        write_string("<Bad format string: ");
        write_hex((unsigned int)fmt);
        write_string(" :");
        for( i = 0; i < 8; i++ )
        {
            write_char(' ');
            val = va_arg(ap, unsigned long);
            write_hex(val);
        }
        write_string(">\n");
        return NO_ERROR;
    }
    /**  */
    while ((c = *fmt++) != '\0')
    {
        if (c == '%')
        {
            c = *fmt++;
            left_prec = right_prec = pad_on_right = islong = islonglong = 0;
            if (c == '-')
            {
                c = *fmt++;
                pad_on_right++;
            }
            if (c == '0')
            {
                zero_fill = TRUE;
                c = *fmt++;
            } else {
                zero_fill = FALSE;
            }
            while( M_COBRA_IS_DIGIT(c) )
            {
                left_prec = (left_prec * 10) + (c - '0');
                c = *fmt++;
            }
            if (c == '.')
            {
                c = *fmt++;
                zero_fill++;
                while( M_COBRA_IS_DIGIT(c) )
                {
                    right_prec = (right_prec * 10) + (c - '0');
                    c = *fmt++;
                }
            }
            else
            {
                right_prec = left_prec;
            }
            sign = '\0';
            if (c == 'l')
            {
                // 'long' qualifier
                c = *fmt++;
		        islong = 1;
                if (c == 'l')
                {
                    // long long qualifier
                    c = *fmt++;
                    islonglong = 1;
                }
            }
            // Fetch value [numeric descriptors only]
            switch (c)
            {
            case 'p':
		      islong = 1;
            case 'd':
            case 'D':
            case 'x':
            case 'X':
            case 'u':
            case 'U':
            case 'b':
            case 'B':
                if (islonglong)
                {
                    val = va_arg(ap, long long);
                }
                else if (islong)
                {
                    val = (long long)va_arg(ap, long);
                }
                else
                {
                    val = (long long)va_arg(ap, int);
                }
                if ((c == 'd') || (c == 'D'))
                {
                    if (val < 0)
                    {
                        sign = '-';
                        val = -val;
                    }
                }
                else
                {
                    // Mask to unsigned, sized quantity
                    if (islong)
                    {
                        val &= ((unsigned long long)1
                                    << (sizeof(unsigned long) * 8)) - 1;
                    }
                    else
                    {
                        val &= ((unsigned long long)1
                                    << (sizeof(unsigned int) * 8)) - 1;
                    }
                }
                break;
            default:
                break;
            }
            // Process output
            switch (c)
            {
            case 'p':  // Pointer
                (*putc)('0');
                (*putc)('x');
                zero_fill = TRUE;
                left_prec = sizeof(unsigned long)*2;
                length = cvt(val, buf, 16, "0123456789abcdef");
                cp = buf;
                break;
            case 'd':
            case 'D':
            case 'u':
            case 'U':
                length = cvt(val, buf, 10, "0123456789");
                cp = buf;
                break;
            case 'x':
                length = cvt(val, buf, 16, "0123456789abcdef");
                cp = buf;
                break;
            case 'X':
                length = cvt(val, buf, 16, "0123456789ABCDEF");
                cp = buf;
                break;
            case 's':
            case 'S':
                cp = va_arg(ap, char *);
                if (cp == NULL)
                    cp = "<null>";
                else if (!check_string(cp))
                {
                    write_string("<Not a string: 0x");
                    write_hex((unsigned int)cp);
                    cp = ">";
                }
                length = 0;
                while (cp[length] != '\0') length++;
                break;
            case 'c':
            case 'C':
                c = va_arg(ap, int /*char*/);
                (*putc)(c);
                res++;
                continue;
            case 'b':
            case 'B':
                length = left_prec;
                if (left_prec == 0)
                {
                    if (islonglong)
                        length = sizeof(unsigned long)*8;
                    else if (islong)
                        length = sizeof(long)*8;
                    else
                        length = sizeof(int)*8;
                }
                for (i = 0;  i < length-1;  i++)
                {
                    buf[i] = ((val & ((unsigned long)1<<i)) ? '1' : '.');
                }
                cp = buf;
                break;
            case '%':
                (*putc)('%');
                break;
            default:
                (*putc)('%');
                (*putc)(c);
                res += 2;
            }
            pad = left_prec - length;
            if (sign != '\0')
            {
                pad--;
            }
            if (zero_fill)
            {
                c = '0';
                if (sign != '\0')
                {
                    (*putc)(sign);
                    res++;
                    sign = '\0';
                }
            }
            else
            {
                c = ' ';
            }
            if (!pad_on_right)
            {
                while (pad-- > 0)
                {
                    (*putc)(c);
                    res++;
                }
            }
            if (sign != '\0')
            {
                (*putc)(sign);
                res++;
            }
            while (length-- > 0)
            {
                c = *cp++;
                (*putc)(c);
                res++;
            }
            if (pad_on_right)
            {
                while (pad-- > 0)
                {
                    (*putc)(' ');
                    res++;
                }
            }
        }
        else
        {
            (*putc)(c);
            res++;
        }
    }
    return (res);
}

/******************************************************************************/
/* EOF */
