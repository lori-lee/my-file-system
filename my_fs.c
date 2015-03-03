#include "flash.h"
#include "config.h"
#include "my_fs.h"
#include "my_string.h"
#include "my_alloc.h"
#include "error.h"
int32 _little_big_endian ()
{
    typedef union _tmp {
        unsigned short i;
        char cs[2];
    }tmp;
    tmp t;
    t.i = 0x0100;
    return t.cs[0];
}
int32 _write_page_offset (int32 page_no, int32 offset, const void *p_addr, int32 len)
{
    if (len <= 0) return E_INVALID_PARAM;    
    flash_write ((page_no << PAGE_SCALE) + offset, (unsigned char *)p_addr, len); 
    return len;
}
int32 _write_page (int32 page_no, const void *p_addr, int32 len)
{
    return _write_page_offset (page_no, 0, p_addr, PAGE_SIZE);
}
void *_read_page_offset (int32 page_no, int32 offset, void *p_buffer, int32 len)
{
    if (len <= 0) return p_buffer;

    flash_read ((page_no << PAGE_SCALE) + offset, (unsigned char *)p_buffer, len);

    return p_buffer;
}
void *_read_page (int32 page_no, void *p_page_buffer)
{
    return _read_page_offset (page_no, 0, p_page_buffer, PAGE_SIZE);
}
int32 _write_super_page (super_page *p_super_page)
{
    p_super_page->checksum = _checksum ((const char *)&(p_super_page->version), PAGE_SIZE - 1);
    _convert_super_page_byte_order (p_super_page, _little_big_endian (), p_super_page->byte_order);
 
    return _write_page (IDX_SUPER_PAGE, (const char *)p_super_page, PAGE_SIZE);
}
void *_convert_directory_byte_order (directory *p_dir, int32 from, int32 to)
{
    if (from != to) {
        convert_byte_order (&(p_dir->parent_filesize), sizeof (int32), from, to);
        convert_byte_order (&(p_dir->sibling), sizeof (int32), from, to);
        convert_byte_order (&(p_dir->first_child_lastpage), sizeof (int32), from, to);
        convert_byte_order (&(p_dir->first_data_page), sizeof (int32), from, to);
        convert_byte_order (&(p_dir->create_datetime), sizeof (int32), from, to);
    }
    return p_dir;
}
super_page *_convert_super_page_byte_order (super_page *p_super_page, int32 from, int32 to)
{
    int32 i;

    if (from != to) {
        convert_byte_order (&(p_super_page->total_pages), sizeof (int32), from, to);
        convert_byte_order (&(p_super_page->create_datetime), sizeof (int32), from, to);
        convert_byte_order (&(p_super_page->pages_alloc_table), sizeof (int32), from, to);
        convert_byte_order (&(p_super_page->first_idle_page), sizeof (int32), from, to);
        convert_byte_order (&(p_super_page->last_idle_page), sizeof (int32), from, to);
        convert_byte_order (&(p_super_page->idle_pages_num), sizeof (int32), from, to);
        convert_byte_order (&(p_super_page->directory), sizeof (int32), from, to);
        convert_byte_order (&(p_super_page->directory_last), sizeof (int32), from, to);
        for (i = 0; i < PAT_PRE_INT_NUM; ++i) {
            convert_byte_order (p_super_page->PAT_start + i, sizeof (int32), from, to);
        }
    }
    return p_super_page;
}
void *_convert_page_byte_order (void *p_page_addr, int32 from, int32 to)
{
    int32 i, *p_iaddr;

    if (from != to) {
        p_iaddr = (int32 *)p_page_addr;
        for (i = 0; i < PAGE_INT_NUM; ++i) {
            convert_byte_order ((char *)p_iaddr + i, sizeof (int32), from, to);
        }
    }
    return p_page_addr;
}
int32 _calc_page_offset_in_PAT (int32 *page_index)
{
    int32 page;

    if (*page_index < PAT_PRE_INT_NUM) {
        return IDX_SUPER_PAGE;
    }
    page = ((*page_index -= PAT_PRE_INT_NUM) + PAGE_INT_NUM) >> (PAGE_SCALE - INT_BYTE_SCALE);
    *page_index &= PAGE_INT_NUM - 1;
    return page + IDX_SUPER_PAGE;
}
static int32 _get_total_pages ()
{
    return PAGES_OCCUPIED (_get_flash_bytes ());
}
/**
 * Calculate the pages would be occupied for secondary allocation table.
 * 
 * @return <int32> pages
 **/
static int32 _calc_PAT_pages ()
{
    static int32 i_PAT_pages = -1;
    if (i_PAT_pages >= 0) return i_PAT_pages;
    i_PAT_pages = (_get_total_pages () - PAT_PRE_INT_NUM + PAGE_INT_NUM - 1) >> (PAGE_SCALE - INT_BYTE_SCALE);
    return i_PAT_pages;
}
static int32 _init_page_link_list (int32 *p_iaddr, int32 start, int32 count)
{
    while (count-- > 0) {
        *p_iaddr++ = start++;
    }
    return start;
}
static void _init_PAT_table_pre ()
{
    int32 i, *p_PAT_start, t, i_cnt_PAT_pre, i_pages_PAT, i_cnt_dir;

    p_PAT_start = (int32 *)PAT_START_P + IDX_SUPER_PAGE;
    (*p_PAT_start++) = PAGE_SUPER;
    i_cnt_PAT_pre = PAT_PRE_INT_NUM - 1 - IDX_SUPER_PAGE;
    i_pages_PAT   = _calc_PAT_pages () - 1 - IDX_SUPER_PAGE;
    i_cnt_dir     = _get_reserved_direcotry_pages ();
    //
    my_imemset (p_PAT_start, PAGE_NULL, i_cnt_PAT_pre);
    //allocation table
    my_imemset ((void *)p_PAT_start, PAGE_ALLOC, MIN(i_pages_PAT, i_cnt_PAT_pre));
    //partial directory page index located inside the super page
    _init_page_link_list (p_PAT_start + i_pages_PAT, IDX_SUPER_PAGE + i_pages_PAT + 1, MIN(i_cnt_dir, i_cnt_PAT_pre - i_pages_PAT));
    my_imemset (p_PAT_start + i_pages_PAT + i_cnt_dir - 1, PAGE_NULL, MIN(1, i_cnt_PAT_pre - i_pages_PAT - i_cnt_dir + 1));

    //partial data page index located inside the super page.
    _init_page_link_list (p_PAT_start + i_pages_PAT + i_cnt_dir, g_p_super_page->first_idle_page + 1, i_cnt_PAT_pre - i_pages_PAT - i_cnt_dir);
    my_imemset (p_PAT_start + _get_total_pages () - 1 - (IDX_SUPER_PAGE + 1), PAGE_NULL, PAT_PRE_INT_NUM - _get_total_pages ());
}
static int32 _calc_PAT_page_end ()
{
    int32 i_PAT_pages;

    i_PAT_pages = _calc_PAT_pages () + IDX_SUPER_PAGE;

    return _calc_page_offset_in_PAT (&i_PAT_pages);
}
static int32 _calc_dir_page_end ()
{
    int32 i_dir_page_end;

    i_dir_page_end = _calc_PAT_pages () + _get_reserved_direcotry_pages () + IDX_SUPER_PAGE;

    return _calc_page_offset_in_PAT (&i_dir_page_end);
}
static void _init_PAT_table_remained ()
{
    int32 *p_PAT_page, i, i_cnt_dir, page, offset, i_last_page_offset;

    p_PAT_page = (int32 *)g_buffer_page;
    offset = _calc_PAT_pages () + IDX_SUPER_PAGE;
    //calcuate the max page NO & offset occupied by page allocation table mark(PAGE_ALLOC)
    page = _calc_page_offset_in_PAT (&offset);
    for (i = IDX_SUPER_PAGE + 1; i < page; ++i) {
        my_imemset (p_PAT_page, PAGE_ALLOC, PAGE_INT_NUM);
        _write_page (i, (const char *)p_PAT_page, PAGE_SIZE);
    }
    if (page > IDX_SUPER_PAGE) {
        //
        my_imemset (p_PAT_page, PAGE_NULL, PAGE_INT_NUM);

        my_imemset (p_PAT_page, PAGE_ALLOC, offset + 1);

        i_cnt_dir   = _get_reserved_direcotry_pages ();
        //
        _init_page_link_list (p_PAT_page + offset + 1, IDX_SUPER_PAGE + _calc_PAT_pages () + 2, MIN(i_cnt_dir, PAGE_INT_NUM - offset - 1));
        my_imemset (p_PAT_page + offset + i_cnt_dir, PAGE_NULL, MIN(1, PAGE_INT_NUM - offset - i_cnt_dir));

        _init_page_link_list (p_PAT_page + offset + i_cnt_dir + 1, IDX_SUPER_PAGE + _calc_PAT_pages () + i_cnt_dir + 2, PAGE_INT_NUM - offset - i_cnt_dir - 1);
        i_last_page_offset = _get_total_pages () - IDX_SUPER_PAGE - 1;
        if (page == _calc_page_offset_in_PAT (&i_last_page_offset)) {
            my_imemset (p_PAT_page + i_last_page_offset, PAGE_NULL, 1);
        }
        
        _write_page (page, (const char *)p_PAT_page, PAGE_SIZE);
    }
    DESTROY_SUPER_PAGE ();
}
static void _init_directory_link_list ()
{
    int32 i, *p_dir_page, i_cnt_dir, start_no, i_dir_page_start, page, offset, i_last_page_offset;

    p_dir_page = (int32 *)g_buffer_page;
    i_cnt_dir  = _get_reserved_direcotry_pages ();
    //
    i_dir_page_start = _calc_PAT_page_end () + 1;

    start_no = ((i_dir_page_start - 1 - IDX_SUPER_PAGE) << (PAGE_SCALE - INT_BYTE_SCALE)) + PAT_PRE_INT_NUM + 1;
    offset = IDX_SUPER_PAGE + _calc_PAT_pages () + i_cnt_dir;
    page   = _calc_page_offset_in_PAT (&offset);
    //
    for (i = i_dir_page_start; i < page; ++i) {
        start_no = _init_page_link_list (p_dir_page, start_no, PAGE_INT_NUM);
        _write_page (i, (const char *)p_dir_page, PAGE_SIZE);
    } 
    if (page >= i_dir_page_start) {
        my_imemset (p_dir_page, PAGE_NULL, PAGE_INT_NUM);
        //
        start_no = _init_page_link_list (p_dir_page, start_no, offset + 1);
        my_imemset (p_dir_page + offset, PAGE_NULL, MIN(1, PAGE_INT_NUM - offset - 1));

        _init_page_link_list (p_dir_page + offset + 1, start_no, PAGE_INT_NUM - offset - 1);

        i_last_page_offset = _get_total_pages () - 1 - IDX_SUPER_PAGE;
        if (page == _calc_page_offset_in_PAT (&i_last_page_offset)) {
            my_imemset (p_dir_page + i_last_page_offset, PAGE_NULL, 1);
        }
        //
        _write_page (page, (const char *)p_dir_page, PAGE_SIZE);
    }
    DESTROY_SUPER_PAGE ();
}
static void _init_idle_page_link_list ()
{
    int32 i, i_idle_page_start, i_idle_offset_start, i_idle_page_end, i_idle_offset_end, *p_idle_page, start_no;

    i_idle_page_start  = _calc_dir_page_end () + 1;

    i_idle_offset_end= _get_total_pages () - 1 - IDX_SUPER_PAGE;
    i_idle_page_end  = _calc_page_offset_in_PAT (&i_idle_offset_end);


    p_idle_page = (int32 *)g_buffer_page;
    start_no= ((i_idle_page_start - 1 - IDX_SUPER_PAGE) << (PAGE_SCALE - INT_BYTE_SCALE)) + PAT_PRE_INT_NUM + 1;

    for (i = i_idle_page_start; i < i_idle_page_end; ++i) {
        start_no = _init_page_link_list (p_idle_page, start_no, PAGE_INT_NUM);
        _write_page (i, (const char *)p_idle_page, PAGE_SIZE);
    }
    _init_page_link_list (p_idle_page, start_no, i_idle_offset_end);
    my_imemset (p_idle_page + i_idle_offset_end, PAGE_NULL, 1);
    _write_page (i_idle_page_end, (const char *)p_idle_page, PAGE_SIZE);

    DESTROY_SUPER_PAGE ();
}
static char _checksum (const char *p_mem, int32 len)
{
    char xor = 0;
    while (len-- > 0) {
        xor ^= *p_mem++;
    }
    return xor;
}
static int32 _init_super_page ()
{
    g_p_super_page = (super_page *)g_buffer_super_page;
    //
    g_p_super_page->version   = MY_FS_VER;
    g_p_super_page->byte_order= _little_big_endian ();
    g_p_super_page->page_size = (unsigned char)PAGE_SCALE;
    //MyFS
    g_p_super_page->magic[0] = 0x4d;
    g_p_super_page->magic[1] = 0x79;
    g_p_super_page->magic[2] = 0x46;
    g_p_super_page->magic[3] = 0x53;

    g_p_super_page->total_pages = _get_flash_bytes () >> PAGE_SCALE;
    
    g_p_super_page->create_datetime  = _get_current_datetime ();
    g_p_super_page->pages_alloc_table= _calc_PAT_pages ();

    g_p_super_page->first_idle_page= _calc_PAT_pages () + _get_reserved_direcotry_pages () + + IDX_SUPER_PAGE + 1;
    g_p_super_page->last_idle_page = _get_total_pages () - 1; 
    g_p_super_page->idle_pages_num = g_p_super_page->last_idle_page - g_p_super_page->first_idle_page + 1;
    g_p_super_page->directory      = IDX_SUPER_PAGE + _calc_PAT_pages () + 1;
    g_p_super_page->directory_last = g_p_super_page->directory + _get_reserved_direcotry_pages () - 1;
    //
    _init_PAT_table_pre ();

    return _write_super_page (g_p_super_page);
}
int32 is_FS_exist ()
{
    INIT_SUPER_PAGE ();
    if (my_strncmp ((const void *)(g_p_super_page->magic), (const void *)"MyFS", 4)) {
        DESTROY_SUPER_PAGE ();
        return 0;
    }
    if (_checksum ((const char *)g_buffer_page, PAGE_SIZE)) {
        DESTROY_SUPER_PAGE ();
        return E_FS_CHCK;
    }
    return 1;
}
void setup_FS ()
{
    _init_super_page ();
    _init_PAT_table_remained ();
    _init_directory_link_list ();
    _init_idle_page_link_list ();
    _create_root_dir ();
}
static int32 _create_root_dir ()
{
    int32 dir_no;

    dir_no = _find_fist_idle_directory (0);
    assert (!dir_no);
    g_p_current_dir = (directory *)g_buffer_directory; 
    my_memset (g_p_current_dir, 0, sizeof (directory));
    NODE_MARK_TYPE_P (g_p_current_dir, TYPE_DIR);
    NODE_MARK_USED_P (g_p_current_dir);
    g_p_current_dir->parent_filesize = PAGE_NULL;
    g_p_current_dir->sibling = g_p_current_dir->first_child_lastpage =PAGE_NULL;
    g_p_current_dir->create_datetime = _get_current_datetime ();
    DESTROY_SUPER_PAGE ();

    dir_no = _write_directory_node_value (dir_no, g_p_current_dir);
    if (dir_no < 0) return dir_no;
    g_i_current_dir_index = dir_no;

    return 0;
}
static int32 _normalize_path (char *p_dir_name)
{
    int32 start, last, i;
    char *p_tmp = p_dir_name;
    
    last = my_strlen (p_dir_name) - 1;
    for (start = 0; ' ' == p_dir_name[start]; ++start) ;
    for (; ' ' == p_dir_name[last]; --last) ;
    while (last >= 0 && '/' == p_dir_name[last]) --last;
    if (start > last) return E_INVALID_PARAM;
    i = start;
    *p_dir_name++ = p_tmp[i++];
    while (i <= last) {
        if ('/' != p_tmp[i]
            || '/' != *(p_dir_name - 1)) {
            *p_dir_name++ = p_tmp[i];
        }
        ++i;
    }
    *p_dir_name = '\0';
    return p_dir_name - p_tmp;
}
int32 open_file (char *file_name, int32 create_if_not_exist)
{
    int32 len, i_parent_dir_no, i_file_name_start, i_dot_index;

    directory *_p_directory_node;
    char _tmp_filename[I_NAME_LEN + I_EXT_LEN + 1];

    if ((len = _normalize_path (file_name)) <= 0) return E_INVALID_PARAM;
    DESTROY_SUPER_PAGE ();
    _p_directory_node = (directory *)g_buffer_directory;

    if ((i_parent_dir_no = _get_parent_directory_value_by_path_name (file_name, _p_directory_node)) < 0) return i_parent_dir_no;

    i_file_name_start = MAX (0, my_str_rpos ("/", file_name, 1, len)) + 1;
    if (len - i_file_name_start > I_NAME_LEN + I_EXT_LEN + 1) return E_INVALID_PARAM;

    my_strncpy (file_name + i_file_name_start, _tmp_filename, len - i_file_name_start);
    i_parent_dir_no = _get_child_directory_by_name (i_parent_dir_no, _tmp_filename, len - i_file_name_start, TYPE_FILE, _p_directory_node);
    if (PAGE_NULL == i_parent_dir_no) {
        if (!create_if_not_exist) return E_NO_FILE;
        if ((i_parent_dir_no = _find_fist_idle_directory (1)) < 0) return i_parent_dir_no;
        i_dot_index = my_str_rpos (".", file_name, 1, len - i_file_name_start);
        my_memset (_p_directory_node, NULL, sizeof (directory));

        my_strncpy (_p_directory_node->name, file_name + i_file_name_start, MIN(I_NAME_LEN, i_dot_index - i_file_name_start));
        my_strncpy (_p_directory_node->extension, file_name + i_dot_index, MIN(I_EXT_LEN, len - i_dot_index - 1));
        NODE_MARK_TYPE_P (_p_directory_node, TYPE_FILE);
        NODE_MARK_USED_P (_p_directory_node);

        _p_directory_node->sibling = _p_directory_node->first_child_lastpage = _p_directory_node->first_data_page;
        _p_directory_node->create_datetime = _get_current_datetime ();

        if ((len = _write_directory_node_value (i_parent_dir_no, _p_directory_node)) < 0) return len;
    }
    return i_parent_dir_no;
}
int32 get_file_size (char *file_name)
{
    int32 i_file_node_index;

    i_file_node_index = open_file (file_name, 0);
    if (i_file_node_index < 0) return i_file_node_index;

    return _get_file_size (i_file_node_index);
}
static int32 _get_real_page_index (int32 i_page_link_list_head, int32 i_page_offset)
{
    while (i_page_offset-- > 0 && i_page_link_list_head >= 0) {
        i_page_link_list_head = _get_next_page (i_page_link_list_head);
    }
    return i_page_link_list_head;
}
int32 _write_aligned_by_page  (const void *p_addr, int32 offset, int32 len)
{
}
int32 _append_file (int32 dir_no, directory *p_file_dir, const void *p_addr, int32 len)
{
    int32 i_first_page_alloc = PAGE_NULL, i_pre_page_no = PAGE_NULL, i_page_offset, i_remained_bytes, _len = len;

    if (len <= 0) return 0;
    INIT_SUPER_PAGE ();
    if (g_p_super_page->idle_pages_num < PAGES_OCCUPIED (len)) return E_LACK_SPACE;
    i_page_offset = p_file_dir->parent_filesize & ~(PAGE_SIZE - 1);
    if (i_page_offset) {
        i_remained_bytes = PAGE_SIZE - i_page_offset;
        if (_write_page_offset (p_file_dir->first_child_lastpage, i_page_offset, p_addr, MIN (len, i_remained_bytes)) < 0) {
            return E_WT;
        }
        p_addr += MIN (len, i_remained_bytes);
        p_file_dir->parent_filesize += MIN (len, i_remained_bytes);
        len   -= MIN (len, i_remained_bytes);
    }
    while (len > 0) {
        if ((i_page_offset = _alloc_page (1, &i_remained_bytes)) < 0) {
            _free_page (i_remained_bytes);
            return i_page_offset;
        }
        if ((i_page_offset = _write_page (i_remained_bytes, p_addr, MIN (PAGE_SIZE, len))) < 0) goto error;
        if ((i_page_offset = _mark_page (i_remained_bytes, PAGE_NULL)) < 0) goto error;
        if (PAGE_NULL != i_first_page_alloc) i_first_page_alloc = i_remained_bytes;
        if (PAGE_NULL != i_pre_page_no) {
            if ((i_page_offset = _mark_page (i_pre_page_no, i_remained_bytes)) < 0) goto error;
        }
        p_addr += PAGE_SIZE;
        p_file_dir->parent_filesize += PAGE_SIZE;
        len -= MIN (len, PAGE_SIZE);
        i_pre_page_no = i_remained_bytes;
    }
    if (PAGE_NULL != i_first_page_alloc) {
        if ((i_page_offset = _mark_page (p_file_dir->first_child_lastpage, i_first_page_alloc)) < 0) return i_page_offset;
        p_file_dir->first_child_lastpage = i_remained_bytes;
    }
    if ((i_page_offset = _write_directory_node_value (dir_no, p_file_dir)) < 0) return i_page_offset;
    return _len;
error:
    _free_page (i_remained_bytes);
    return i_page_offset;
}
int32 _overwrite_file_offset (directory *p_file_dir, int32 i_page_start, int32 i_page_offset, const void *p_addr, int32 len)
{
    int32 i_rt_code, _l;

    len = _l = MIN (p_file_dir->parent_filesize, len);
    while (len > 0) {
        if ((i_rt_code = _write_page_offset (i_page_start, i_page_offset, p_addr, MIN (len, PAGE_SIZE - i_page_offset))) < 0) return i_rt_code;
        p_addr += MIN (len, PAGE_SIZE - i_page_offset);
        len -= MIN (len, PAGE_SIZE - i_page_offset);
        i_page_offset = 0;
        i_page_start = _get_next_page (i_page_start);
    }

    return _l;
}
int32 write_file (int32 i_file_node_index, const void *p_addr, int32 offset, int32 len)
{
    directory _tmp_directory;
    int32 i_page_index, i_page_offset, i_page_real_index, i_rt_code;

    if (len <= 0) return 0;

    _get_directory_node_value (i_file_node_index, &_tmp_directory);
    if (TYPE_FILE != NODE_TYPE (_tmp_directory)
        || NODE_IS_FREE(_tmp_directory) || _tmp_directory.parent_filesize < 0) {
        return E_INVALID_PARAM;
    }
    offset = MAX (MIN (offset, _tmp_directory.parent_filesize - 1), 0);
    if (offset + len + 1 > _tmp_directory.parent_filesize) {
        INIT_SUPER_PAGE ();
        if (g_p_super_page->idle_pages_num < PAGES_OCCUPIED (offset + len + 1 - _tmp_directory.parent_filesize)) return E_LACK_SPACE;
    }

    if (offset < _tmp_directory.parent_filesize) {
        i_page_offset= offset & ~(PAGE_SIZE - 1);
        if (!offset) {
            i_page_real_index = _tmp_directory.first_data_page;
        } else {
            i_page_index = offset >> PAGE_SCALE;
            i_page_real_index = _get_page_index_by_link_node_index (_tmp_directory.first_data_page, i_page_index);
        }
    }
    if ((i_rt_code = _overwrite_file_offset (&_tmp_directory, i_page_real_index, i_page_offset, p_addr, len)) < 0) return i_rt_code;
    if ((i_rt_code = _append_file (i_file_node_index, &_tmp_directory, p_addr + _tmp_directory.parent_filesize, len - _tmp_directory.parent_filesize)) < 0) return i_rt_code;

    return len;
}
int32 _rm_dir_node (int32 i_parent_dir_no, char *p_name, int32 type)
{
    directory _tmp_parent_dir, _tmp_curr_dir;
    int32 i_pre_dir_no, i_current_dir_no;

    type &= TYPE_FILE;
    if (PAGE_NULL == i_parent_dir_no || _normalize_path (p_name) <= 0 || !my_strncmp ("/", p_name, 1)) return E_INVALID_PARAM;
    _get_directory_node_value (i_parent_dir_no, &_tmp_parent_dir);

    i_pre_dir_no = PAGE_NULL;
    i_current_dir_no = _tmp_parent_dir.first_child_lastpage;
    while (PAGE_NULL != i_current_dir_no) {
        _get_directory_node_value (i_current_dir_no, &_tmp_curr_dir);
        if (NODE_TYPE (_tmp_curr_dir) == type
            && !my_strncmp (_tmp_curr_dir.name, p_name, my_strlen (p_name))) {
            break;
        }
        i_pre_dir_no = i_current_dir_no;
        i_current_dir_no = _tmp_curr_dir.sibling;
    }
    if (PAGE_NULL == i_current_dir_no) return TYPE_DIR == type ? E_NO_DIR : E_NO_FILE;
    if (PAGE_NULL == i_pre_dir_no) {
             
    } else {
    }
    if (TYPE_DIR == type) {
    }
}
int32 _get_pre_sibling_dir_by_name (char *p_file_name)
{
}
int32 rm_file (char *p_file_name)
{
    int32 i_last_slash_pos, i_parent_dir_no, i_rt_code;
    directory _target_dir;

    if ((i_parent_dir_no = _get_parent_directory_value_by_path_name (p_file_name, &_target_dir)) < 0) return i_parent_dir_no;
    if (!my_strncmp ("/", p_file_name, 1)) return E_INVALID_PARAM;
    i_last_slash_pos = my_str_rpos ("/", p_file_name, 1, my_strlen (p_file_name));
    if ((i_rt_code = _get_child_directory_by_name (i_parent_dir_no, p_file_name + i_last_slash_pos + 1, my_strlen (p_file_name) - i_last_slash_pos, TYPE_FILE, &_target_dir)) < 0) {
        return i_rt_code;
    }
    _get_directory_node_value (i_parent_dir_no, &_target_dir);
}
int32 cd (char *p_dir_name)
{
    int32 i_current_no;

    if ((i_current_no = _get_directory_no_by_name (p_dir_name)) < 0) {
        return i_current_no;
    }
    return g_i_current_dir_index = i_current_no;
}
static int32 _get_directory_no_by_name (char *p_dir_name)
{
    int32 len, i_current_dir_no, i_slash_start = 0, i_slash_end;
    char _tmp_name[I_NAME_LEN + I_EXT_LEN + 1];
    
    if ((len = _normalize_path (p_dir_name)) < 0) return len;
    if ('/' == *p_dir_name) {
        i_current_dir_no = 0;
        ++i_slash_start;
    } else i_current_dir_no = g_i_current_dir_index; 
    assert (i_current_dir_no >= 0);
    //
    while (i_slash_start < len) {
        i_slash_end = my_str_pos ("/", p_dir_name, i_slash_start, 1, len - i_slash_start);
        if (PAGE_NULL == i_slash_end) i_slash_end = len - 1;
        if (i_slash_end - i_slash_start > I_NAME_LEN + I_EXT_LEN) return E_INVALID_PARAM;
        my_strncpy (p_dir_name + i_slash_start, _tmp_name, i_slash_end - i_slash_start);
        if (!my_strncmp (".", _tmp_name, i_slash_end - i_slash_start)) {
            //DO NOTHING
        } else if (!my_strncmp ("..", _tmp_name, i_slash_end - i_slash_start)) {
            if (i_current_dir_no) {
                i_current_dir_no = _get_directory_parent_index (i_current_dir_no);
            }
        } else {
            i_current_dir_no = _get_child_dir_no_by_name (i_current_dir_no, _tmp_name, i_slash_end - i_slash_start, TYPE_DIR);
        }
        if (i_current_dir_no < 0) {
            return i_current_dir_no;
        }
        i_slash_start = i_slash_end + 1;
    }
    return i_current_dir_no;
}
static int32 _get_parent_directory_value_by_path_name (char *p_dir_name, directory *p_parent_directory)
{
    int32 len, i_current_dir_no, i_slash_pos_start = 0, i_slash_pos_end, i_parent_path_end_pos;
    char _tmp_name[I_NAME_LEN + I_EXT_LEN];

    if ((len = _normalize_path (p_dir_name)) <= 0) return E_INVALID_PARAM;
    if ('/' == *p_dir_name) {
        ++i_slash_pos_start;
        i_current_dir_no = 0;
    } else i_current_dir_no = g_i_current_dir_index;
    assert (i_current_dir_no >= 0);
    i_parent_path_end_pos = my_str_rpos ("/", p_dir_name, 1, len);

    //
    if (i_parent_path_end_pos > i_slash_pos_start) {//check parent path & change to the directory parent
        do {
            i_slash_pos_end = my_str_pos ("/", p_dir_name, i_slash_pos_start, 1, i_parent_path_end_pos - i_slash_pos_start);
            if (i_slash_pos_end - i_slash_pos_start > I_NAME_LEN + I_EXT_LEN) return E_INVALID_PARAM;
            my_strncpy (p_dir_name, _tmp_name, i_slash_pos_end - i_slash_pos_start);
            if (!my_strncmp (".", _tmp_name, i_slash_pos_end - i_slash_pos_start)) {
                //DO NOTHING
            } else if (!my_strncmp ("..", _tmp_name, i_slash_pos_end - i_slash_pos_start)) {
                if (i_current_dir_no) {
                    i_current_dir_no = _get_directory_parent_index (i_current_dir_no);
                }
            } else {
                i_current_dir_no = _get_child_directory_by_name (i_current_dir_no, _tmp_name, i_slash_pos_end - i_slash_pos_start, TYPE_DIR, p_parent_directory);
            }
            if (i_current_dir_no < 0) return E_NO_DIR;
            i_slash_pos_start = i_slash_pos_end + 1;
        } while (i_slash_pos_end < i_parent_path_end_pos);
    } else {
        _get_directory_node_value (i_current_dir_no, p_parent_directory);
    }
    return i_current_dir_no;
}
int32 create_dir (char *p_dir_name)
{
    int32 i_new_dir_no, i_first_child_dir_no, i_current_dir_no, i_slash_pos_start = 0, i_slash_pos_end, r;
    directory _tmp_current_dir;
    char _tmp_name[I_NAME_LEN + I_EXT_LEN];

    if ((i_current_dir_no = _get_parent_directory_value_by_path_name (p_dir_name, &_tmp_current_dir)) < 0) return i_current_dir_no;
    i_slash_pos_start = my_str_rpos ("/", p_dir_name, 1, i_slash_pos_end = my_strlen (p_dir_name));
    if (PAGE_NULL == i_slash_pos_start) i_slash_pos_start = 0;
    else ++i_slash_pos_start;

    if (i_slash_pos_end - i_slash_pos_start > I_NAME_LEN + I_EXT_LEN) return E_INVALID_PARAM;
    my_strncpy (p_dir_name + i_slash_pos_start, _tmp_name, i_slash_pos_end - i_slash_pos_start);
    if (r = _is_sub_dir_exists (i_current_dir_no, _tmp_name, TYPE_DIR, i_slash_pos_end - i_slash_pos_start)) return r;

    i_new_dir_no = _find_fist_idle_directory (1);
    if (i_new_dir_no < 0) return i_new_dir_no;

    i_first_child_dir_no = _tmp_current_dir.first_child_lastpage;
    my_memset (&_tmp_current_dir, 0, sizeof (directory));
    my_strncpy (_tmp_name, _tmp_current_dir.name, i_slash_pos_end - i_slash_pos_start);
    NODE_MARK_TYPE (_tmp_current_dir, TYPE_DIR);
    _tmp_current_dir.parent_filesize = i_current_dir_no;
    _tmp_current_dir.sibling= i_first_child_dir_no;
    _tmp_current_dir.first_child_lastpage = PAGE_NULL;
    _tmp_current_dir.first_data_page = PAGE_NULL;
    _tmp_current_dir.create_datetime = _get_current_datetime ();

    if ((r = _write_directory_node_value (i_new_dir_no, &_tmp_current_dir)) < 0) return r;
    if ((r = _write_directory_firstchild_field (i_current_dir_no, i_new_dir_no)) < 0) return r;

    return i_new_dir_no;
}
int32 rm_dir (char *p_dir_name)
{
    int32 len, i_parent_dir_no, i_target_dir_no, i_pre_dir_no;
    directory _tmp_dir_node;
    char _tmp_name[I_NAME_LEN + I_EXT_LEN + 1];

    if ((len = _normalize_path (p_dir_name)) <= 0) return E_INVALID_PARAM;
    if ((i_parent_dir_no = _get_parent_directory_value_by_path_name (p_dir_name, &_tmp_dir_node)) < 0) return i_parent_dir_no;

    my_strncpy (_tmp_dir_node.name, _tmp_name, I_NAME_LEN + I_EXT_LEN);
    _tmp_name[I_NAME_LEN + I_EXT_LEN] = '\0';

    if ((i_target_dir_no = _get_child_directory_by_name (i_parent_dir_no, _tmp_name, my_strlen (_tmp_name), TYPE_DIR, &_tmp_dir_node)) < 0) {
        return i_parent_dir_no;
    }
    if (PAGE_NULL != _tmp_dir_node.first_child_lastpage) return E_DIR_NOT_EMPTY;
    if (PAGE_NULL == (i_pre_dir_no = _get_pre_sibling_dir_no (i_parent_dir_no))) return E_INVALID_PARAM;
    else if (i_pre_dir_no < 0) return E_NO_DIR;
    _write_directory_sibling_field (i_pre_dir_no, _get_directory_sibling_index (i_target_dir_no));

    NODE_MARK_IDLE (_tmp_dir_node);
    _tmp_dir_node.sibling = PAGE_NULL;

    return _write_directory_node_value (i_target_dir_no, &_tmp_dir_node);
}
int32 get_cwd (char *p_addr)
{
    int32 i_current_dir_no, i_current_len = 0, i_delta_len, i_slash_start, i_slash_end;

    i_current_dir_no = g_i_current_dir_index;
    while (PAGE_NULL != i_current_dir_no) {
        _get_directory_node_name (i_current_dir_no, p_addr + i_current_len);
        i_delta_len = my_strlen (p_addr + i_current_len);
        i_current_len += i_delta_len;
        p_addr[i_current_len++] = '/';
        i_current_dir_no = _get_directory_parent_index (i_current_dir_no);
    }
    my_str_reverse (p_addr, 0, i_current_len);
    for (i_slash_start = 1; i_slash_start < i_current_len; i_slash_start = i_slash_end + 1) {
        i_slash_end = my_str_pos ("/", p_addr, i_slash_start, 1, i_current_len - i_slash_start);
        if (PAGE_NULL == i_slash_end) i_slash_end = i_current_len;
        my_str_reverse (p_addr + i_slash_start, 0, i_slash_end - i_slash_start);
    }
    return 0;
}
int32 list_next_child (char *p_dir_name, char **p_buff)
{
    static int32 i_sdir_no = PAGE_NULL, i_scurr_child = PAGE_NULL;
    static char name[I_NAME_LEN + I_EXT_LEN + 1];
    int32 i_len, i_curr_dir_no;
    directory _tmp_dir;

    if ((i_len = _normalize_path (p_dir_name)) <= 0) return E_INVALID_PARAM;
    if ((i_curr_dir_no =  _get_directory_no_by_name (p_dir_name)) < 0) return i_curr_dir_no;
    if (i_sdir_no != i_curr_dir_no) { 
        _get_directory_node_value (i_curr_dir_no, &_tmp_dir);
        i_scurr_child = _tmp_dir.first_child_lastpage;
        i_sdir_no = i_curr_dir_no;
    }
    if (PAGE_NULL != i_scurr_child) {
        _get_directory_node_value (i_scurr_child, &_tmp_dir);
        my_memset (name, 0, I_NAME_LEN + I_EXT_LEN + 1);
        my_strncpy (_tmp_dir.name, name, I_NAME_LEN + I_EXT_LEN + 1);
        *p_buff = name;
        i_scurr_child = _tmp_dir.sibling;
        return 0;
    } else return PAGE_NULL;
}
#ifdef _DEBUG_
#undef _DEBUG_
int32 main (void)
{
    int32 page_no, page_index;
    char buff[] = "  //////////path_A//pathB///////a/filename.ext   ";

    my_console ("_little_big_endian:%d\n", _little_big_endian ());

    page_index = 155;
    page_no = _calc_page_offset_in_PAT (&page_index);
    my_console ("page index:%d:%d, total pages:%d, PAT pages:%d\n", page_no, page_index, _get_total_pages (), _calc_PAT_pages ());
    _normalize_path (buff);
    my_console ("first:%s\n", buff);
    _normalize_path (buff);
    my_console ("Second:%s\n", buff);
    setup_FS ();
    return 0;
}
#endif
