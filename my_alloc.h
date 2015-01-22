#include "config.h"
#include "my_fs.h"
#ifndef _MY_FS_ALLOC_H_
#define _MY_FS_ALLOC_H_

/**
 * Get the next pageNO followed page [page_no]
 *
 * @param <int32> page_no
 *
 * @return <in> > 0 (next sibling pageNO), PAGE_NULL (no following, ), < 0 (error)
 *
 **/
int32 _get_next_page (int32 page_no);
/**
 * Try to allocate n pages, and the page no allocated will be stored in p_i_page_no
 *
 * @param <int32> n  pages to be allocated
 * @param <int32 *> p_i_page_no  pages' page no allocated
 *
 * @return <in> >= 0 (success), < 0 (error occured)
 *
 **/
int32 _alloc_page (int32 n, int32 *p_i_page_no);
/**
 * set page_no reside in PAT to be v
 *
 * @param <int32> page_no
 * @param <int32> v
 *
 * @return <int32> > 0 (success) < 0 (error>
 *
 **/
int32 _mark_page (int32 page_no, int32 v);
/**
 * Free page with page index page_no
 *
 * @param <int32> page_no the page to be free
 *
 * @return <int32> > 0 (success) < 0 (error)
 *
 **/
int32 _free_page (int32 page_no);
/**
 * calcuate the related page no and offset of directory dir_no
 *
 * @param <int32 *> dir_no  directory node index (based on 0)
 *
 * @return <int32> page no & *dir_no will be updated to the offset inside the page
 *
 **/
static int32 _calc_dir_target_page (int32 *dir_no);
/**
 * Fetch the value of directory with index dir_no
 *
 * @param <int32> dir_no (based on 0)
 * @param <directory *> p_dir the address to store the directory node's value
 *
 * @return <directory *>p_dir
 **/
extern directory *_get_directory_node_value (int32 dir_no, directory *p_dir);
/**
 * Write the directory contents to the directory with index dir_no
 *
 * @param <int32> dir_no
 * @param <directory *> p_dire the contents to be written
 *
 * @return <int32> >0 (success), < 0 (error)
 *
 **/
extern int32 _write_directory_node_value (int32 dir_no, directory *p_dir);
static int32 _write_directory_node_field (int32 dir_no, void *p_buff, int32 offset, int32 bytes);
extern int32 _write_directory_name_field (int32 dir_no, void *p_buff, int32 len);
extern int32 _write_directory_type (int32 dir_no, char type);
extern int32 _write_directory_parent_field (int32 dir_no, int32 i_parent);
extern int32 _write_directory_sibling_field (int32 dir_no, int32 i_sibling_no);
extern int32 _write_directory_firstchild_field (int32 dir_no, int32 i_firstchild_lastpage);
extern int32 _write_directory_first_data_page_field (int32 dir_no, int32 first_data_page);
extern int32 _write_directory_create_datetime_field (int32 dir_no, int32 create_datetime);
/**
 *
 * Get the pre-sibling's directory no
 *
 * @param <int32> i_dir_no current directory node
 *
 * @return <int32>
 **/
int32 _get_pre_sibling_dir_no (int32 i_dir_no);
/**
 * Find the first idle directory node located in directory link list
 *
 * @param <int32> alloc specify whether to try to allocate a new page or not when no idle node exist
 *
 * @return <int32> the idle node index
 *
 **/
extern int32 _find_fist_idle_directory (int32 alloc);
/**
 * Get the root directory node value
 *
 * @param <directory *> p_directory the value to be stored
 *
 * @param <int32> >= 0 (the root directory node index) <0 (error occured)
 **/
static int32 _get_root_directory_node_value (directory *p_directory);
static int32 _get_directory_node_field_value (int32 i_dir_no, void *p_buff, int32 offset, int32 bytes);
extern int32 _get_child_directory_index (int32 i_parent_dir_no);
extern int32 _get_directory_sibling_index (int32 i_dir_no);
extern int32 _get_directory_parent_index (int32 i_dir_no);
extern void *_get_directory_node_name (int32 i_dir_no, void *p_buffer);
extern int32 _get_directory_type (int32 i_dir_no);
extern int32 _get_page_index_by_link_node_index (int32 link_list_head, int32 link_list_node_offset);

extern int32 _get_child_dir_no_by_name (int32 i_parent_dir_no, const char *p_name, int32 len, int32 type);
int32 _get_file_size (int32 i_file_node_index);
extern int32 _is_sub_dir_exists (int32 i_parent_dir_no, const char *p_name, int32 len, int32 type);
/**
 *
 * Get the  children directory node value by its name
 *
 * @param <int32> i_parent_dir_no
 * @param <const char *p_name> directory node's name
 * @param <int32> len the bytes of p_name
 * @param <int32> type the node type (file or directory)
 * @param <directory *> p_directory the place where to store the directory value
 *
 * @param <int32> >= 0 (the related directory node index) < 0 (error occured)
 *
 **/
extern int32 _get_child_directory_by_name (int32 i_parent_dir_no, const char *p_name, int32 len, int32 type, directory *p_directory);
/**
 * get total bytes remained in flash disk
 *
 * @return <int32> bytes remained
 **/
extern int32 get_free_size ();
#endif
