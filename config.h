#ifndef _MY_CONFIG_H_
#define _MY_CONFIG_H_

#define _DEBUG_
//total memory size(bytes)
#define MEM_SIZE (10 * 1024)
//page size
#define PAGE_SCALE     8
#define PAGE_SIZE (1 << PAGE_SCALE)
//offset of super page (index starts from 0)
#define IDX_SUPER_PAGE 1

#define INT_BYTES 4

#if INT_BYTES == 4
    #define INT_BYTE_SCALE 2
#elif INT_BYTES == 2
    #define INT_BYTE_SCALE 1
#elif INT_BYTES == 1
    #define INT_BYTE_SCALE 0
#endif

static int _get_flash_bytes ()
{
    return 1 << 20;
}
static int _get_current_datetime ()
{
    return 0;
}
static int _get_reserved_direcotry_pages ()
{
    return 1 << 6;
}
#endif
