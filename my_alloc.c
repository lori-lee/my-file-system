#include "my_fs.h"
#include "my_string.h"
#include "error.h"
#include "my_alloc.h"
int get_free_size ()
{
    if (is_FS_exist ()) {
        return g_p_super_page->idle_pages_num * PAGE_SIZE;
    } else return -1;
}
int _get_next_page (int page_no)
{
    int page, byte_order, v;
    INIT_SUPER_PAGE ();
    byte_order = BYTE_ORDRE_STORED;
    if (page_no < 0 || page_no >= g_p_super_page->total_pages) return E_INVALID_PARAM;
    page = _calc_page_offset_in_PAT (&page_no);
    if (IDX_SUPER_PAGE == page) {
        return PAT_OFFSET_GET_V (page_no);
    } else {
        _read_page_offset (page, page_no, (void *)&v, sizeof (int));
        
        return *(int *)convert_byte_order (&v, sizeof (int), byte_order, _little_big_endian ());
    }
}
int _alloc_page (int n, int *p_i_page_no)
{
    int i, start;
    
    if (n < 0) return E_INVALID_PARAM;
    if (!n) return 0;
    INIT_SUPER_PAGE ();
    if (g_p_super_page->idle_pages_num < n) return E_LACK_SPACE;
    start = g_p_super_page->first_idle_page;
    p_i_page_no[0] = start;
    for (i = 1; i < n; ++i) {
        p_i_page_no[i] = _get_next_page (p_i_page_no[i - 1]);
        if (p_i_page_no[i] <= 0) return E_DISK_VALUE;
    }
    g_p_super_page->idle_pages_num -= n;
    g_p_super_page->first_idle_page = _get_next_page (p_i_page_no[i - 1]);
    if (g_p_super_page->first_idle_page) return E_DISK_VALUE;
    i = _mark_page (p_i_page_no[i - 1], PAGE_NULL);
    if (i < 0) return i;
    i = _write_super_page (g_p_super_page);
    if (i < 0) return i;
    return n;
}
static int _mark_page (int page_no, int v)
{
    int offset, page;
    INIT_SUPER_PAGE ();
    if (page_no < 0 || page_no >= g_p_super_page->total_pages) return E_INVALID_PARAM;
    offset = page_no;
    page = _calc_page_offset_in_PAT (&offset);
    if (IDX_SUPER_PAGE == page) {
        PAGE_OFFSET_SET_V_P (g_p_super_page, offset, int, page_no, int);
        return _write_super_page (g_p_super_page);
    }
    _convert_page_byte_order (&v, _little_big_endian (), BYTE_ORDRE_STORED);
    return _write_page_offset (page, offset, (const void *)&v, sizeof (int)); 
}
static int _free_page (int page_no)
{
    int i;

    if (page_no <= 0) return E_INVALID_PARAM;
    i = _mark_page (page_no, PAGE_NULL);
    if (i < 0) return i;

    INIT_SUPER_PAGE ();
    i = _mark_page (g_p_super_page->last_idle_page, page_no);
    if (i < 0) return i;
    g_p_super_page->last_idle_page = page_no;
    ++g_p_super_page->idle_pages_num;
    i = _write_super_page (g_p_super_page);
    if (i < 0) return i;
    return page_no;
}
static int _calc_dir_target_page (int *dir_no)
{
    int i_dir_start_page, i_page_target_offset, i;

    INIT_SUPER_PAGE ();
    i_dir_start_page = g_p_super_page->directory;
    i_page_target_offset = *dir_no / DIR_NODE_NUM_PER_PAGE;
    *dir_no %= DIR_NODE_NUM_PER_PAGE;
    for (i = 0; i < i_page_target_offset; ++i) {
        i_dir_start_page = _get_next_page (i_dir_start_page);
    }
    return i_dir_start_page;
}
directory *_get_directory_node_value (int dir_no, directory *p_dir)
{
    int page;
    char byte_order;

    INIT_SUPER_PAGE ();
    byte_order = BYTE_ORDRE_STORED;
    page = _calc_dir_target_page (&dir_no);
    _read_page_offset (page, dir_no * sizeof (directory), (void *)p_dir, sizeof (directory));
    _convert_directory_byte_order (p_dir, byte_order, _little_big_endian ());
    return (directory *)p_dir;
}
int _write_directory_node_value (int dir_no, directory *p_dir)
{
    int page;

    INIT_SUPER_PAGE ();
    _convert_directory_byte_order (p_dir, _little_big_endian (), BYTE_ORDRE_STORED);
    page = _calc_dir_target_page (&dir_no);

    return _write_page_offset (page, dir_no * sizeof (directory), (void *)p_dir, sizeof (directory));
}
static int _write_directory_node_field (int dir_no, void *p_buff, int offset, int bytes)
{
    char byte_order;
    int page;

    INIT_SUPER_PAGE ();
    byte_order = BYTE_ORDRE_STORED;
    page = _calc_dir_target_page (&dir_no);
    if (offset + bytes > DIRECTORY_FIELD_OFFSET (parent_filesize)) {
        if (offset <= DIRECTORY_FIELD_OFFSET (parent_filesize)) {
            convert_byte_order (p_buff + DIRECTORY_FIELD_OFFSET (parent_filesize) - offset, bytes + offset - DIRECTORY_FIELD_OFFSET (parent_filesize),  _little_big_endian (), byte_order);
        } else {
            convert_byte_order (p_buff, bytes, _little_big_endian (), byte_order);
        }
    }

    return _write_page_offset (page, dir_no * sizeof (directory) + offset, p_buff, bytes);
}
int _write_directory_name_field (int dir_no, void *p_buff, int len)
{
    if (len > I_NAME_LEN + I_EXT_LEN) return E_INVALID_PARAM;

    return _write_directory_node_field (dir_no, p_buff, DIRECTORY_FIELD_OFFSET (name), len);
}
int _write_directory_type (int dir_no, char type)
{
    return _write_directory_node_field (dir_no, &type, DIRECTORY_FIELD_OFFSET (type), sizeof (char));
}
int _write_directory_parent_field (int dir_no, int i_parent)
{
    return _write_directory_node_field (dir_no, &i_parent, DIRECTORY_FIELD_OFFSET (parent_filesize), sizeof (int));
}
int _write_directory_sibling_field (int dir_no, int i_sibling_no)
{
    return _write_directory_node_field (dir_no, &i_sibling_no, DIRECTORY_FIELD_OFFSET (sibling), sizeof (int));
}
int _write_directory_firstchild_field (int dir_no, int i_firstchild_lastpage)
{
    return _write_directory_node_field (dir_no, &i_firstchild_lastpage, DIRECTORY_FIELD_OFFSET (first_child_lastpage), sizeof (int));
}
int _write_directory_first_data_page_field (int dir_no, int first_data_page)
{
    return _write_directory_node_field (dir_no, &first_data_page, DIRECTORY_FIELD_OFFSET (first_data_page), sizeof (int));
}
int _write_directory_create_datetime_field (int dir_no, int create_datetime)
{
    return _write_directory_node_field (dir_no, &create_datetime, DIRECTORY_FIELD_OFFSET (create_datetime), sizeof (int));
}
int _find_fist_idle_directory (int alloc)
{
    int start, i, j, k, n;
    char type;

    INIT_SUPER_PAGE ();
    start = g_p_super_page->directory;
    for (i = 0; start != PAGE_NULL; ++i) {
        for (j = 0; j < DIR_NODE_NUM_PER_PAGE; ++j) {
            _read_page_offset (start, sizeof (directory) * j + DIRECTORY_FIELD_OFFSET (type), (void *)&type, sizeof (char));
            if (!(type & NODE_OCCUPIED_MASK)) {
                return i * DIR_NODE_NUM_PER_PAGE + j;
            }
        }
        start = _get_next_page (start);
    }
    if (alloc) {
        k = _alloc_page (1, &n);
        if (k < 0) return k;
        _mark_page (n, PAGE_NULL);
        _mark_page (g_p_super_page->directory_last, n);
        g_p_super_page->directory_last = n;
        k = _write_super_page (g_p_super_page);
        if (k < 0) return k;

        return i * DIR_NODE_NUM_PER_PAGE;
    }
    return PAGE_NULL;
}
int _get_pre_sibling_dir_no (int i_dir_no)
{
    int start, i, j, sibling, no;

    if (!i_dir_no) return PAGE_NULL;
    INIT_SUPER_PAGE ();
    start = g_p_super_page->directory;
    no    = 0;
    for (i = 0; PAGE_NULL != start; ++i) {
        for (j = 0; j < DIR_NODE_NUM_PER_PAGE; ++j, ++no) {
            sibling = _get_directory_sibling_index (no);
            if (sibling == i_dir_no) return no; 
        }
        start = _get_next_page (start);
    }
    return E_INVALID_PARAM;
}

static int _get_root_directory_node_value (directory *p_directory)
{
    _get_directory_node_value (0, p_directory);
    DESTROY_SUPER_PAGE ();
    if (PAGE_NULL != p_directory->parent_filesize) return PAGE_NULL;
    return 0;
}
static int _get_directory_node_field_value (int i_dir_no, void *p_buff, int offset, int bytes)
{
    int page;
    char byte_order;

    assert (i_dir_no >= 0
            && offset >=0
            && offset + bytes <= sizeof (directory));

    INIT_SUPER_PAGE ();
    byte_order = BYTE_ORDRE_STORED;
    page = _calc_dir_target_page (&i_dir_no);
    _read_page_offset (page, i_dir_no * sizeof (directory) + offset, p_buff, bytes);
    if (offset + bytes > DIRECTORY_FIELD_OFFSET (parent_filesize)) {
        if (offset <= DIRECTORY_FIELD_OFFSET (parent_filesize)) {
            convert_byte_order (p_buff + DIRECTORY_FIELD_OFFSET (parent_filesize) - offset, bytes + offset - DIRECTORY_FIELD_OFFSET (parent_filesize), byte_order, _little_big_endian ());
        } else {
            convert_byte_order (p_buff, bytes, byte_order, _little_big_endian ());
        }
    }
    return bytes;
}
static char _get_directory_type (int i_dir_no)
{
    char type;

    _get_directory_node_field_value (i_dir_no, &type, DIRECTORY_FIELD_OFFSET (type), sizeof (char));

    return type & ~(NODE_OCCUPIED_MASK - 1);
}
int _get_child_directory_index (int i_parent_dir_no)
{
    int i_child_no;

    _get_directory_node_field_value (i_parent_dir_no, &i_child_no, DIRECTORY_FIELD_OFFSET (first_child_lastpage), sizeof (int));
    return i_child_no;
}
int _get_directory_sibling_index (int i_dir_no)
{
    int i_sibling_no;

    _get_directory_node_field_value (i_dir_no, &i_sibling_no, DIRECTORY_FIELD_OFFSET (sibling), sizeof (int));
    return i_sibling_no;
}
int _get_directory_parent_index (int i_dir_no)
{
    int i_parent_no;

    _get_directory_node_field_value (i_dir_no, &i_parent_no, DIRECTORY_FIELD_OFFSET (parent_filesize), sizeof (int));
    return i_parent_no;
}
void *_get_directory_node_name (int i_dir_no, void *p_buffer)
{
    _get_directory_node_field_value (i_dir_no, p_buffer, DIRECTORY_FIELD_OFFSET (name), DIRECTORY_FIELD_OFFSET (type));
    return p_buffer;
}
int _get_file_size (int i_file_node_index)
{
    _get_directory_parent_index (i_file_node_index);
}
int _is_sub_dir_exists (int i_parent_dir_no, const char *p_name, int len, int type)
{
    directory _tmp_dir;
    int r;

    r = _get_child_directory_by_name (i_parent_dir_no, p_name, len, type, &_tmp_dir);
    if (r < 0) return r;
    else if (r > 0) return r;
    else return 0;
}
int _get_child_directory_by_name (int i_parent_dir_no, const char *p_name, int len, int type, directory *p_directory)
{
    int child, r, i_dot_index;

    if (i_parent_dir_no < 0 || TYPE_DIR != type || TYPE_FILE != type) return E_INVALID_PARAM;
    child = _get_child_directory_index (i_parent_dir_no);
    i_dot_index = my_str_rpos (".", p_name, 1, r = my_strlen (p_name));
    if ((TYPE_FILE == type && r > I_NAME_LEN + I_EXT_LEN + 1)
        || (TYPE_DIR == type && r > I_NAME_LEN + I_EXT_LEN)) return E_INVALID_PARAM;

    _get_directory_node_field_value (i_parent_dir_no, &r, DIRECTORY_FIELD_OFFSET (type), sizeof (char));
    if (TYPE_DIR != r) return E_INVALID_PARAM;

    while (PAGE_NULL != child) {
        _get_directory_node_value (child, p_directory);
        if (type == NODE_TYPE_P (p_directory)) {
            if (TYPE_DIR == type) {
                if (!my_strncmp (p_name, p_directory->name, MIN(len, I_NAME_LEN + I_EXT_LEN))) {
                    return child;
                }
            } else {
                if (i_dot_index >= 0) {
                    if (!(r = my_strncmp (p_name, p_directory->name, MIN(i_dot_index, I_NAME_LEN)))) {
                        r = my_strncmp (p_name + i_dot_index + 1, p_directory->extension, MIN(len - i_dot_index - 1, I_EXT_LEN));
                    }
                } else {
                    r = my_strncmp (p_name, p_directory->name, MIN(len, I_NAME_LEN));
                }
                if (!r) return child;
            }
        }
        child = p_directory->sibling;
    }
    return PAGE_NULL;
}
