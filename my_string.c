#include "config.h"
#include "my_string.h"
#include "my_fs.h"
#include "error.h"

#ifdef _DEBUG_
#include <stdio.h>
#include <stdlib.h>
#endif
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
void *convert_byte_order (void *p_addr, int32 len, int32 from, int32 to)
{
    int32 i, j;

    char *p_a, *p_b;	

    if (from != to) {
        for (i = 0, j = len - 1; i < j; ++i, --j) {
            p_a = (char *)p_addr + i;
            p_b = (char *)p_addr + j;
            if (*p_a != *p_b) {
                *p_a ^= *p_b; *p_b ^= *p_a; *p_a ^= *p_b;
            }
        }
    }
    return p_addr;
}
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
void *convert_byte_order_by_unit (void *p_addr, int32 unit, int32 len, int32 from, int32 to)
{
    char *p_a;

    if (from != to) {
        p_a = (char *)p_addr;
        while (len >= unit) {
            convert_byte_order (p_a, unit, from, to);
            p_a += unit;
            len -= unit;
        }
        convert_byte_order (p_a, len, from, to);
    }
    return p_addr;
}
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
void *my_strncpy (const void *p_addr_src, void *p_addr_dest, int32 len)
{
    char *p_dest = (char *)p_addr_dest;

    if (len <= 0 || p_addr_src == p_addr_dest) return p_addr_dest;
    if (p_addr_src < p_addr_dest && p_addr_src + len > p_addr_dest) {//copy from the tail
        p_addr_src += len - 1;
        p_addr_dest+= len - 1;
        while (len--) *(char *)p_addr_dest-- = *(const char *)p_addr_src--;
    } else {//copy from head
        while (len--) *(char *)p_addr_dest++ = *(const char *)p_addr_src++;
    }
    return p_dest;
}
/**
 * Compare two string by byte, like strncmp
 * 
 * @param <const void *> p_pattern the address of the first string
 * @param <const void *> p_subject the address of the second string
 * @param <int32> len the length of bytes to be compared
 *
 * @return <int32> < 0(p_pattern < p_subject) = 0 or > 0
 **/
int32 my_strncmp (const void *p_pattern, const void *p_subject, int32 len)
{
    const char *p_str_pt, *p_str_sub;

    p_str_pt = (const char *)p_pattern;
    p_str_sub= (const char *)p_subject;
    while (len > 0) {
        if (*p_str_pt != *p_str_sub) return *p_str_pt - *p_str_sub;
        else {
            --len;
            ++p_str_pt; ++p_str_sub;
        }
    }
    return 0;
}
/**
 * like libc memset
 * @param <void *> p_mem
 * @param <int32> len
 *
 **/
void *my_memset (void *p_mem, char ch, int32 len)
{
    int32 i, t, l, *p_iaddr;
    char *p_addr;

    if (len > 0) {
        p_iaddr = (int32 *)p_mem;
        t = (ch << 24) | (ch << 16) | (ch << 8) | ch;
        l = len >> 2;
        for (i = 0; i < l; ++i) *p_iaddr++ = t;
        p_addr = (char *)p_iaddr;
        for (i = 0, l = len & 3; i < l; ++i) *p_addr++ = ch;
    }
    return p_mem;
}
/**
 * like _memset, but the unit of filling is int32, not char
 * @param <void *> p_mem
 * @param <int32> v
 * @param <int32> len
 *
 **/
void *my_imemset (void *p_mem, int32 v, int32 len)
{
    int32 i, *p_iaddr;
    
    if (len > 0) {
        p_iaddr = (int32 *)p_mem;
        for (i = 0; i < len; ++i) *p_iaddr++ = v;
    }
    return p_mem;
}
/**
 * Like str_rpos
 * @param <const char *> p_pattern
 * @param <const char *> p_subject
 * @param <int32> len_pattern
 * @param <int32> len_subject
 *
 * @return <int32>
 **/
int32 my_str_rpos (const char *p_pattern, const char *p_subject, int32 len_pattern, int32 len_subject)
{
    const char *p_t_subject = p_subject + len_subject - len_pattern;

    if (len_pattern > len_subject) return PAGE_NULL;
    while (p_t_subject >= p_subject) {
        if (!my_strncmp (p_pattern, p_t_subject, len_pattern)) return p_t_subject - p_subject;
        --p_t_subject;
    }
    return PAGE_NULL;
}
int32 my_str_pos (const char *p_pattern, const char *p_subject, int32 start, int32 len_pattern, int32 len_subject)
{
    if (start < 0 || len_pattern < 0 || len_subject < 0) return E_INVALID_PARAM;
    len_subject -= len_pattern;
    while (start < len_subject) {
        if (!my_strncmp (p_pattern, p_subject + start, len_pattern)) return start;
        ++start;
    }
    return PAGE_NULL;
}
int32 my_strlen (const char *p_str)
{
    int32 len = 0;

    while (*p_str++) len++;
    return len;
}
int32 my_itoa (int32 n, void *p_buff)
{
    int32 i = 0, j = n;

    if (n < 0) n = -n;
    do {
        *(char *)(p_buff + i++) = (n % 10) | 0x30;
        n /= 10;
    } while (n);
    if (j < 0) *(char *)(p_buff + i++) = '-';
    convert_byte_order (p_buff, i, 0, 1);
    return i;
}
int32 my_itohexa (int32 n, void *p_buff)
{
    int32 i = 0, j = n;

    if (n < 0) n = -n;
    do {
        if ((n & 15) >= 10) *(char *)(p_buff + i++) = (n & 15) - 10 + 'A';
        else *(char *)(p_buff + i++) = (n & 15) | 0x30;
        n >>= 4;
    } while (n);
    if (j < 0) *(char *)(p_buff + i++) = '-';
    convert_byte_order (p_buff, i, 0, 1);
    return i;
}
static void _my_puts (const char *p_str, int32 start, int32 len)
{
#ifdef _DEBUG_
    while (len-- > 0) printf ("%c", p_str[start++]);
#else
    while (len-- > 0)  putchar (p_str[start++]);
#endif
}
static void _my_putchar (char ch)
{
    putchar (ch);
}
void my_console (const char *p_str_format, ...)
{
    int32 status = 0, i = 0, n;
    char tmp[INT_BYTES], ch;
    const char *p_str;

    va_list var;
    va_start (var, p_str_format);
    while (p_str_format[i]) {
        switch (status) {
        case 0x0://
            if ('%' == p_str_format[i]) {
                status = 1;
            } else if ('\\' == p_str_format[i]) {
                status = 2;
            } else _my_putchar (p_str_format[i]); break;
        case 0x1:
            switch (p_str_format[i]) {
            case 'c':
                if (sizeof (void *) > sizeof (int32)) {//64 bit
                    ch = va_arg (var, int32);
                } else ch = va_arg (var, int);
                _my_putchar (ch); break;
            case 's':
                p_str = va_arg (var, const char *);
                _my_puts (p_str, 0, my_strlen (p_str)); break;
            case 'd':
                n = va_arg (var, int32);
                n = my_itoa (n, tmp);
                _my_puts (tmp, 0, n); break;
            case 'x':
            case 'p':
                n = my_itohexa (va_arg (var, int32), tmp);
                _my_puts (tmp, 0, n);
                break;
            case '%':
            default:
                _my_putchar (p_str_format[i]);
                break;
            }
            status ^= 1;
            break;
        }
        ++i;
    }
    va_end (var);
}
void my_str_reverse (char *p_str, int32 offset, int32 len)
{
    int32 i, j;
    
    for (i = offset, j = offset + len - 1; i < j; ++i, --j) {
        if (p_str[i] != p_str[j]) {
            p_str[i] ^= p_str[j]; p_str[j] ^= p_str[i]; p_str[i] ^= p_str[j];
        }
    }
}
#ifdef _DEBUG_MY_STRING_
int32 main (void)
{
    char buff[13] = "abcdefghijkl";

    my_console ("%c-%d-%s-0X%x\n", 'A', 123456, "hello string", 731);
    my_console ("ABLen:%d\n", my_strlen ("ABLen"));
    my_console ("acb<=>fuck[ab]end:%d\n", my_str_rpos("acb", "fuck[ab]end", 2, 11));
    convert_byte_order (buff, 12, 0, 1);
    my_console ("%s\n", buff);
    convert_byte_order_by_unit (buff, 4, 12, 0, 1);
    my_console ("%s\n", buff);
    my_strncpy (buff, buff + 4, 8);
    my_console ("%s\n", buff);

    my_console ("pattern:subject[pattern][pattern]end:%d\n", my_str_pos ("pattern", "subject[pattern][pattern]end", 0, my_strlen ("pattern"), my_strlen ("subject[pattern][pattern]end")));
    my_console ("pattern:subject[pattern][pattern]end:%d\n", my_str_pos ("pattern", "subject[pattern][pattern]end", 8, my_strlen ("pattern"), my_strlen ("subject[pattern][pattern]end")));
    my_console ("pattern:subject[pattern][pattern]end:%d\n", my_str_pos ("pattern", "subject[pattern][pattern]end", 16, my_strlen ("pattern"), my_strlen ("subject[pattern][pattern]end")));
    my_console ("pattern:subject[pattern][pattern]end:%d\n", my_str_pos ("pattern", "subject[pattern][pattern]end", 18, my_strlen ("pattern"), my_strlen ("subject[pattern][pattern]end")));
    return 0;
}
#endif
