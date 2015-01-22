#include "config.h"
int32 _get_flash_bytes ()
{
    return (int32)1 << 20;
}
int32 _get_current_datetime ()
{
    return 0;
}
int32 _get_reserved_direcotry_pages ()
{
    return (int32)1 << 6;
}

