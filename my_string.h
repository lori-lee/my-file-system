#include "config.h"
#ifndef _MY_STRING_H_
#define _MY_STRING_H_

#include <stdarg.h>
#ifndef va_start
    typedef void *va_list;
    #define _INTSIZE(type) ((sizeof (type) + sizeof (int32) - 1) & ~(sizeof (int32) - 1))
    #define va_start(ap, v)  (ap = ((va_list)&v) + _INTSIZE(v))
    #define va_arg(ap, type) (*(type *)((ap += _INTSIZE (type)) - _INTSIZE (type)))
    #define va_end(ap) (void *)0
#endif
//extern void putchar (int32 ch);
/**
 *
 * Convert the byte order from type [from] to [to]
 *
 * @param <void *> p_addr contents to be converted
 * @param <int32> len bytes to be converted
 * @param <enum> from (LITTLE_ENDIAN | BIG_ENDIAN)
 * @param <enum> to (LITTLE_ENDIAN | BIG_ENDIAN)
 *
 * @return <void *> p_addr
 *
 **/
extern void *convert_byte_order (void *p_addr, int32 len, int32 from, int32 to);
/**
 *
 * Convert the byte order by unit from type [from] to [to]
 *
 * @param <void *> p_addr contents to be converted
 * @param <int32> unit
 * @param <int32> len bytes to be converted
 * @param <enum> from (LITTLE_ENDIAN | BIG_ENDIAN)
 * @param <enum> to (LITTLE_ENDIAN | BIG_ENDIAN)
 *
 * @return <void *> p_addr
 *
 **/
extern void *convert_byte_order_by_unit (void *p_addr, int32 unit, int32 len, int32 from, int32 to);
/**
 * like strncpy
 *
 * @param <const void *>p_addr_src
 * @param <void *>p_addr_dest
 * @param <int32> len
 *
 * @return <void *>p_addr_dest
 *
 **/
extern void *my_strncpy (const void *p_addr_src, void *p_addr_dest, int32 len);
/**
 * Compare two string by byte, like strncmp
 * 
 * @param <const void *> p_pattern the address of the first string
 * @param <const void *> p_subject the address of the second string
 * @param <int32> len the length of bytes to be compared
 *
 * @return <int32> < 0(p_pattern < p_subject) = 0 or > 0
 **/
extern int32 my_strncmp (const void *p_pattern, const void *p_subject, int32 len);
/**
 * like libc memset
 * @param <void *> p_mem
 * @param <int32> len
 *
 **/
extern void *my_memset (void *p_mem, char ch, int32 len);
/**
 * like _memset, but the unit of filling is int32, not char
 * @param <void *> p_mem
 * @param <int32> v
 * @param <int32> len
 *
 **/
extern void *my_imemset (void *p_mem, int32 v, int32 len);
/**
 * Like str_rpos
 * @param <const char *> p_pattern
 * @param <const char *> p_subject
 * @param <int32> len_pattern
 * @param <int32> len_subject
 *
 * @return <int32>
 **/
extern int32 my_str_rpos (const char *p_pattern, const char *p_subject, int32 len_pattern, int32 len_subject);
/**
 * Find the first match index that greater than or equal to start
 *
 * @param <const char *> p_pattern
 * @param <const char *> p_subject
 * @param <int32> start position to start searching
 * @param <int32> len_pattern
 * @param <int32> len_subject
 *
 * @return <int32> >= 0 or -1 (not found)
 *
 **/
int32 my_str_pos (const char *p_pattern, const char *p_subject, int32 start, int32 len_pattern, int32 len_subject);
/**
 * Like strlen
 *
 **/
extern int32 my_strlen (const char *p_str);
/**
 *
 * Like itoa
 *
 **/
extern int32 my_itoa (int32 n, void *p_buff);
/**
 * Convert an integer to a hex string.
 *
 **/
extern int32 my_itohexa (int32 n, void *p_buff);
/**
 *
 * Similar to printf
 *
 **/
extern void my_console (const char *p_str_format, ...);
extern void my_str_reverse (char *p_str, int32 start, int32 len);
#endif
