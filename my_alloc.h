#ifndef _MY_FS_ALLOC_H_
#define _MY_FS_ALLOC_H_

/**
 * Get the next pageNO followed page [page_no]
 *
 * @param <int> page_no
 *
 * @return <in> > 0 (next sibling pageNO), PAGE_NULL (no following, ), < 0 (error)
 *
 **/
int _get_next_page (int page_no);
/**
 * Try to allocate n pages, and the page no allocated will be stored in p_i_page_no
 *
 * @param <int> n  pages to be allocated
 * @param <int *> p_i_page_no  pages' page no allocated
 *
 * @return <in> >= 0 (success), < 0 (error occured)
 *
 **/
int _alloc_page (int n, int *p_i_page_no);
/**
 * set page_no reside in PAT to be v
 *
 * @param <int> page_no
 * @param <int> v
 *
 * @return <int> > 0 (success) < 0 (error>
 *
 **/
static int _mark_page (int page_no, int v);
/**
 * Free page with page index page_no
 *
 * @param <int> page_no the page to be free
 *
 * @return <int> > 0 (success) <= 0 (error)
 *
 **/
static int _free_page (int page_no);
/**
 * calcuate the related page no and offset of directory dir_no
 *
 * @param <int *> dir_no  directory node index (based on 0)
 *
 * @return <int> page no & *dir_no will be updated to the offset inside the page
 *
 **/
static int _calc_dir_target_page (int *dir_no);
/**
 * Fetch the value of directory with index dir_no
 *
 * @param <int> dir_no (based on 0)
 * @param <directory *> p_dir the address to store the directory node's value
 *
 * @return <directory *>p_dir
 **/
extern directory *_get_directory_node_value (int dir_no, directory *p_dir);
/**
 * Write the directory contents to the directory with index dir_no
 *
 * @param <int> dir_no
 * @param <directory *> p_dire the contents to be written
 *
 * @return <int> >0 (success), <= 0 (error)
 *
 **/
extern int _write_directory_node_value (int dir_no, directory *p_dir);
static int _write_directory_node_field (int dir_no, void *p_buff, int offset, int bytes);
extern int _write_directory_name_field (int dir_no, void *p_buff, int len);
extern int _write_directory_type (int dir_no, char type);
extern int _write_directory_parent_field (int dir_no, int i_parent);
extern int _write_directory_sibling_field (int dir_no, int i_sibling_no);
extern int _write_directory_firstchild_field (int dir_no, int i_firstchild_lastpage);
extern int _write_directory_first_data_page_field (int dir_no, int first_data_page);
extern int _write_directory_create_datetime_field (int dir_no, int create_datetime);
/**
 *
 * Get the pre-sibling's directory no
 *
 * @param <int> i_dir_no current directory node
 *
 * @return <int>
 **/
int _get_pre_sibling_dir_no (int i_dir_no);
/**
 * Find the first idle directory node located in directory link list
 *
 * @param <int> alloc specify whether to try to allocate a new page or not when no idle node exist
 *
 * @return <int> the idle node index
 *
 **/
extern int _find_fist_idle_directory (int alloc);
/**
 * Get the root directory node value
 *
 * @param <directory *> p_directory the value to be stored
 *
 * @param <int> >= 0 (the root directory node index) <0 (error occured)
 **/
static int _get_root_directory_node_value (directory *p_directory);
static int _get_directory_node_field_value (int i_dir_no, void *p_buff, int offset, int bytes);
extern int _get_child_directory_index (int i_parent_dir_no);
extern int _get_directory_sibling_index (int i_dir_no);
extern int _get_directory_parent_index (int i_dir_no);
extern void *_get_directory_node_name (int i_dir_no, void *p_buffer);
extern int _get_directory_type (int i_dir_no);

extern int _get_child_dir_no_by_name (int i_parent_dir_no, const char *p_name, int len, int type);
int _get_file_size (int i_file_node_index);
extern int _is_sub_dir_exists (int i_parent_dir_no, const char *p_name, int len, int type);
/**
 *
 * Get the  children directory node value by its name
 *
 * @param <int> i_parent_dir_no
 * @param <const char *p_name> directory node's name
 * @param <int> len the bytes of p_name
 * @param <int> type the node type (file or directory)
 * @param <directory *> p_directory the place where to store the directory value
 *
 * @param <int> >= 0 (the related directory node index) < 0 (error occured)
 *
 **/
extern int _get_child_directory_by_name (int i_parent_dir_no, const char *p_name, int len, int type, directory *p_directory);
/**
 * get total bytes remained in flash disk
 *
 * @return <int> bytes remained
 **/
extern int get_free_size ();
#endif
