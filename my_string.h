#ifndef _MY_STRING_H_
#define _MY_STRING_H_

#include <stdarg.h>
#ifndef va_start
    typedef void *va_list;
    #define _INTSIZE(type) ((sizeof (type) + sizeof (int) - 1) & ~(sizeof (int) - 1))
    #define va_start(ap, v)  (ap = ((va_list)&v) + _INTSIZE(v))
    #define va_arg(ap, type) (*(type *)((ap += _INTSIZE (type)) - _INTSIZE (type)))
    #define va_end(ap) (void *)0
#endif
//extern void putchar (int ch);
/**
 *
 * Convert the byte order from type [from] to [to]
 *
 * @param <void *> p_addr contents to be converted
 * @param <int> len bytes to be converted
 * @param <enum> from (LITTLE_ENDIAN | BIG_ENDIAN)
 * @param <enum> to (LITTLE_ENDIAN | BIG_ENDIAN)
 *
 * @return <void *> p_addr
 *
 **/
extern void *convert_byte_order (void *p_addr, int len, int from, int to);
/**
 *
 * Convert the byte order by unit from type [from] to [to]
 *
 * @param <void *> p_addr contents to be converted
 * @param <int> unit
 * @param <int> len bytes to be converted
 * @param <enum> from (LITTLE_ENDIAN | BIG_ENDIAN)
 * @param <enum> to (LITTLE_ENDIAN | BIG_ENDIAN)
 *
 * @return <void *> p_addr
 *
 **/
extern void *convert_byte_order_by_unit (void *p_addr, int unit, int len, int from, int to);
/**
 * like strncpy
 *
 * @param <const void *>p_addr_src
 * @param <void *>p_addr_dest
 * @param <int> len
 *
 * @return <void *>p_addr_dest
 *
 **/
extern void *my_strncpy (const void *p_addr_src, void *p_addr_dest, int len);
/**
 * Compare two string by byte, like strncmp
 * 
 * @param <const void *> p_pattern the address of the first string
 * @param <const void *> p_subject the address of the second string
 * @param <int> len the length of bytes to be compared
 *
 * @return <int> < 0(p_pattern < p_subject) = 0 or > 0
 **/
extern int my_strncmp (const void *p_pattern, const void *p_subject, int len);
/**
 * like libc memset
 * @param <void *> p_mem
 * @param <int> len
 *
 **/
extern void *my_memset (void *p_mem, char ch, int len);
/**
 * like _memset, but the unit of filling is int, not char
 * @param <void *> p_mem
 * @param <int> v
 * @param <int> len
 *
 **/
extern void *my_imemset (void *p_mem, int v, int len);
/**
 * Like str_rpos
 * @param <const char *> p_pattern
 * @param <const char *> p_subject
 * @param <int> len_pattern
 * @param <int> len_subject
 *
 * @return <int>
 **/
extern int my_str_rpos (const char *p_pattern, const char *p_subject, int len_pattern, int len_subject);
/**
 * Find the first match index that greater than or equal to start
 *
 * @param <const char *> p_pattern
 * @param <const char *> p_subject
 * @param <int> start position to start searching
 * @param <int> len_pattern
 * @param <int> len_subject
 *
 * @return <int> >= 0 or -1 (not found)
 *
 **/
int my_str_pos (const char *p_pattern, const char *p_subject, int start, int len_pattern, int len_subject);
/**
 * Like strlen
 *
 **/
extern int my_strlen (const char *p_str);
/**
 *
 * Like itoa
 *
 **/
extern int my_itoa (int n, void *p_buff);
/**
 * Convert an integer to a hex string.
 *
 **/
extern int my_itohexa (int n, void *p_buff);
/**
 *
 * Similar to printf
 *
 **/
extern void my_console (const char *p_str_format, ...);
#endif
