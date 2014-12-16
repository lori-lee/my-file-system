#include "config.h"
#ifndef _MY_FS_H_
#define _MY_FS_H_

#ifdef _DEBUG_
#undef assert
#define assert(v)  do {\
                       if (!(v)) {\
                            my_console ("file:[%s], line:[%d], assert (%s) failed.\n", __FILE__, __LINE__, #v);\
                       }\
                   } while (0)
#elif defined assert
#undef assert
#define assert(v)
#endif
#define MY_FS_VER 0x10 //version majorVer.minVer(xxxx.xxxx)

#define LITTLE_ENDIAN 0
#define BIG_ENDIAN 1

char g_buffer_page[PAGE_SIZE];

#define ALIGN_BY_PAGE(n) (((n) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGES_OCCUPIED(n) (((n) + PAGE_SIZE - 1) >> PAGE_SCALE)

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define PAGE_SUPER -3 //is a page of master allocation table
#define PAGE_ALLOC -2 //is a page of secondary allocation table
#define PAGE_NULL  -1 //the end of a link list
#ifndef NULL
#define NULL 0
#endif

#define PAT_PRE_SIZE (PAGE_SIZE - 8 - (INT_BYTES << 3))
#define PAT_PRE_INT_NUM (PAT_PRE_SIZE >> INT_BYTE_SCALE)

#define PAGE_INT_NUM (PAGE_SIZE >> INT_BYTE_SCALE)
/******************The graph of FS*****************************\
 *+------------------+
 *|                  |
 *|     Page 0       | => the super page (page 0)
 *|-----+------------|-+
 *|     |PAT_start   | |
 *+------------------+ |
 *|                  | |
 *|     Page 1       | |
 *|                  | |
 *|                  | |
 *+------------------+ |
 *|                  | +=> PAT (page 1 ~ page I)
 *|     Page 2       | |
 *|       .          | |
 *|       .          | |
 *+------------------+ |
 *|       .          | |
 *|       .          | |
 *|     Page I       | |
 *|                  | |
 *+------------------+-+
 *|                  | |
 *|     Page I+1     | |
 *|       .          | |
 *|       .          | |
 *+------------------+ +=> Directory node
 *|       .          | |
 *|       .          | |
 *|     Page I+J     | |
 *|                  | |
 *+------------------+-+
 *|                  | |
 *|     Page I+J+1   | |
 *|                  | |
 *|                  | |
 *+------------------+ |
 *|                  | |
 *|     Page I+J+2   | |
 *|       .          | |
 *|       .          | |
 *+------------------+ +=> Data storage part
 *|       .          | |
 *|       .          | |
 *|       .          | |
 *|       .          | |
 *+------------------+ |
 *|                  | |
 *|     Page N       | |
 *|                  | |
 *|                  | |
 *+------------------+-+
 *
 ************************************************************/


/**
 * Super page, 256 bytes.
 *
 **/
typedef struct _super_page {
    unsigned char checksum;//0x0 the xor value of the super block
    unsigned char version;//0x1
    unsigned char byte_order;//0x2, 0 -- little endian, 1 -- big endian
    unsigned char page_size;//0x3 actual page size = 2 ^ pageSize (>= 256 = 2 ^ 8)

    char magic[4];//0x4 -- 0x4d 79 46 53 (MyFS)
    unsigned int total_pages;//0x8 blocks contained of the disk
    unsigned int create_datetime;//0xc the creation datetime of MyFS, year[6bit].month[4bit].day[5bit].hour[5bit].minute[6bit].sec[6bit]
    // 
    unsigned int pages_alloc_table;//0x10 the pages occupied by allocation table
    unsigned int first_idle_page;//0x14 the first unused page no
    unsigned int last_idle_page;//0x18 the last unused page no
    unsigned int idle_pages_num;//0x1c the total number of unused pages
    unsigned int directory;//0x20 the first page NO. of directory
    unsigned int directory_last;//0x24 the last page NO. of direcotry
    int PAT_start[PAT_PRE_INT_NUM];
}super_page;
static super_page *g_p_super_page = NULL;
#define INIT_SUPER_PAGE() do {\
                              if(!g_p_super_page) {\
                                  DESTROY_CWD ();\
                                  _read_page (IDX_SUPER_PAGE, (void *)g_buffer_super_page);\
                                  g_p_super_page = (super_page *)g_buffer_super_page;\
                                  _convert_super_page_byte_order (g_p_super_page, g_p_super_page->byte_order, _little_big_endian ());\
                              }\
                          } while (0)
#define INIT_CWD() do {\
                       assert (g_i_current_dir_index >= 0);\
                       g_i_current_dir_index = g_i_current_dir_index >= 0 ? g_i_current_dir_index : 0;\
                       if (!g_p_current_dir) {\
                           DESTROY_SUPER_PAGE ();\
                           _get_directory_node_value (g_i_current_dir_index, (directory *)g_buffer_directory);\
                           g_p_current_dir = (directory *)g_buffer_directory;\
                       }\
                   } while (0)
#ifndef MEM_SIZE
    #error "MEM_SIZE should be defined first."
#endif
#if MEM_SIZE >= ((PAGE_SIZE >> 1) + (PAGE_SIZE << 1))
    static char g_buffer_super_page[sizeof (super_page)];
    static char g_buffer_directory[sizeof (directory)];
    #define DESTROY_SUPER_PAGE()
    #define DESTROY_CWD()
#else
    static char *g_buffer_super_page = g_buffer_page;
    static char *g_buffer_directory  = g_buffer_page;
    #define DESTROY_SUPER_PAGE() g_p_super_page = NULL 
    #define DESTROY_CWD() g_buffer_directory = NULL
#endif

#define PAGE_OFFSET_SET_V_P(p_page, offset, offset_type, v, v_type) *(v_type *)(((offset_type *)p_page) + offset) = v
#define BYTE_ORDRE_STORED g_p_super_page->byte_order
#define PAT_START_P g_p_super_page->PAT_start
#define PAT_OFFSET_GET_V(offset) g_p_super_page->PAT_start[offset]
#define PAT_OFFSET_SET_V(offset, v) g_p_super_page->PAT_start[offset] = v

#define DATA_PAGE_INDEX(file_offset) ((file_offset) >> PAGE_SCALE)
#define DATA_PAGE_OFFSET(file_offset) ((file_offset) & (PAGE_SIZE - 1))

#define TYPE_DIR  0x0 //directory
#define TYPE_FILE 0x1 //file

#define NODE_TYPE(d) ((d).type & 0x1)
#define NODE_TYPE_P(pd) ((pd)->type & 0x1)

#define NODE_OCCUPIED_MASK 0x80
#define NODE_IS_FREE(d) (!((d).type & NODE_OCCUPIED_MASK))
#define NODE_IS_FREE_P(pd) (!((pd)->type & NODE_OCCUPIED_MASK))

#define NODE_MARK_USED(d) ((d).type |= NODE_OCCUPIED_MASK)
#define NODE_MARK_USED_P(pd) ((pd)->type |= NODE_OCCUPIED_MASK)

#define NODE_MARK_IDLE(d) ((d).type &= NODE_OCCUPIED_MASK - 1)
#define NODE_MARK_IDLE_P(pd) ((pd)->type &= NODE_OCCUPIED_MASK - 1)

#define NODE_MARK_TYPE(d,t) ((d).type &= NODE_OCCUPIED_MASK | t)
#define NODE_MARK_TYPE_P(pd,t) ((pd)->type &= NODE_OCCUPIED_MASK | t)

#define DIRECTORY_FIELD_OFFSET(field) ((long)&(((directory *)0)->field))
typedef struct _directory {
    char name[8];//0x0
    char extension[3];//0x8
    char type;//0xb P.S. when the MSB is set, means the node is used, else not
    int parent_filesize;//0xc node index, for file node, this field record the file length
    int sibling;//0x10 node index
    int first_child_lastpage;//0x14 node index
    int first_data_page;//0x18 first page no of the data stored, only file should set it
    int create_datetime;//0x1c the creation datetime of MyFS, year[6bit].month[4bit].day[5bit].hour[5bit].minute[6bit].sec[6bit]
}directory;
static directory *g_p_current_dir = NULL;
static int g_i_current_dir_index = PAGE_NULL;
#define DIR_NODE_NUM_PER_PAGE  (PAGE_SIZE >> 3)
#define I_NAME_LEN 8
#define I_EXT_LEN  3

//functions declaration
/**
 * return the type of system order byte
 *
 * @return <enum> LITTLE_ENDIAN | BIG_ENDIAN
 **/
extern int _little_big_endian ();
/**
 * write data to the page specified by pageNo
 *
 * @param <int> page_no  the page NO to be written
 * @param <const char*> p_src the page content to be written
 * @param <int> len the content length
 *
 * @return <int> the length written (>=0) or error occurred (< 0)
 *
 **/
extern int _write_page (int page_no, const void *p_src, int len);
/**
 * Write contents to the disk
 * @param <int> page_no the page NO to be written
 * @param <int> offset the page offset to be written
 * @param <const void *> p_src the start address of the contents
 * @param <int> len the length of the content
 *
 * @return <int> (>=0) the length written, or error (< 0)
 **/
extern int _write_page_offset (int page_no, int offset, const void *p_src, int len);
/**
 * Write the super page's content
 * @param <super_page *> p_super_page
 *
 * @return <int> the length written (>=0) or error occurred (< 0)
 **/
extern int _write_super_page (super_page *p_super_page);
/**
 * read the data of a page
 *
 * @param <int>  page_no  the page index (based on 0) to be read
 * @param <void *> p_page_buffer the buffer to store the page content
 *
 * @return <int> the length read (>=0) or error occurred (< 0)
 **/
extern void *_read_page (int page_no, void *p_page_buffer);
/**
 * Read the content specified by page_no and page offset
 * 
 * @param <int> page_no  the page to be read
 * @param <int> offset   the offset(based on 0) relative the beginning of the page
 * @param <void *>p_buffer the buffer to store the content
 * @param <int> len  the length to be read
 * 
 * @return <int> the length read(>=0) or an error occured (< 0)
 *
 **/
extern void *_read_page_offset (int page_no, int offset, void *p_buffer, int len);
/**
 *
 * Convert the byte order of the content of a directory node
 * @param <directory *> p_dir
 * @param <enum> from
 * @param <enum> to
 *
 **/
extern void *_convert_directory_byte_order (directory *p_dir, int from, int to);
/**
 * convert the byte order of super page's content
 * @param <super_page *>  p_super_page the start address of the content
 * @param <enum> from (LITTLE_ENDIAN | BIG_ENDIAN)
 * @param <enum> to (LITTLE_ENDIAN | BIG_ENDIAN)
 *
 * @return <void *> p_super_page
 **/
extern void *_convert_super_page_byte_order (super_page *p_super_page, int from, int to);
/**
 * convert the byte order of the content pointed by p_page_addr
 * @param <void *>  p_page_addr 
 * @param <enum> from (LITTLE_ENDIAN | BIG_ENDIAN)
 * @param <enum> to (LITTLE_ENDIAN | BIG_ENDIAN)
 *
 * @return <void *> p_page_addr
 **/
extern void *_convert_page_byte_order (void *p_page_addr, int from, int to);
/**
 * calculate the PAT offset of page, return page, and *pageIndex will be the offset in the related page.
 * 
 * @param <int> page_no (based on 0)
 *
 * @return <int> page no & page offset will be store back to page_no
 **/		
extern int _calc_page_offset_in_PAT (int *page_no);
/**
 * calculate the pages should be occupied by page allocation table
 *
 * @return <int>
 **/
static int _calc_PAT_pages ();
/**
 * initialize a link list, will set *p_start = start, *(p_start + 1) = start + 1, ..., *(p_start + count - 1) = start + count - 1
 *
 * @param <int *> p_start
 * @param <int> start
 * @param <int> count
 *
 * @return <int> count if success, error (< 0)
 **/
static int _init_page_link_list (int *p_start, int start, int count);
/**
 * initialize the super page
 *
 **/
static int _init_super_page ();
/**
 * calcuate the checksum start from p_mem
 * 
 * @param <const char *> p_mem
 * @param <int> len the bytes to be calculated
 *
 * @return <char>
 **/
static char _checksum (const char *p_mem, int len);
/**
 * initialize the part located at in the super page of page alloc table
 *
 **/
static void _init_PAT_table_pre ();
/**
 * initilize the part followed the super page of page alloc table
 *
 **/
static void _init_PAT_table_remained ();
/**
 *
 * initialize the directory list
 *
 **/
static void _init_directory_link_list ();
/**
 * initialize the pages to be allocated to data storage
 *
 **/
static void _init_idle_page_link_list ();
/**
 *
 * Calcute the last page NO of page alloc table
 *
 * @return <int> the last page NO of PAT
 *
 **/
static int _calc_PAT_page_end ();
/**
 *
 * Calcuate the last page NO of directory
 *
 * @return <int>
 **/
static int _calc_dir_page_end ();
/**
 * Create root directory
 *
 **/
static int _create_root_dir ();
/**************************Functions listed below can be used by external code***************/
/**
 *
 * Setup file system
 *
 **/
extern void setup_FS();
/**
 * Judge whethe file system exists correctly
 *
 * @return <int> > 0 (success), <= 0 (error)
 *
 **/
extern int is_FS_exist ();
/**
 * Get the parent directory by path name
 * 
 * @param <char *> p_dir_name
 * @param <directory *> p_parent_directory parent directory node value will be stored here
 *
 * @return <int> the directory index of parent directory
 *
 **/
static int _get_parent_directory_value_by_path_name (char *p_dir_name, directory *p_parent_directory);
/**
 *
 * Open a file
 *
 * @param <char *> file_name
 * @param <int> create_if_not_exist (if this value is not 0, then a new empty file will be created if it does not exist before)
 *
 * @return <int> > 0 (the directory node index of the file), < 0 (error)
 *
 **/
extern int open_file (char *file_name, int create_if_not_exist);
/**
 * create a file
 *
 * @param <const char *> file_name (e.g. '/path/path/file.ext', 'file.ext')
 * @param <const void *> p_file_data contents of the file
 * @param <int> len length of content's bytes
 *
 * @return <int> > 0 (success), < 0 (error occured)
 *
 **/
extern int create_file (const char *file_name, const void *p_file_data, int len);
/**
 * read a file's content into a buffer
 *
 * @param <const char *> file_name 
 * @param <void *> p_addr buffer the file content
 * @param <int> offset the offset of file to be read
 * @param <int> len bytes to be read
 *
 * @return <int> > 0 (success, may less than len if no more content exists), 0 -- already the file end, < 0 (error)
 *
 **/
extern int read_file (const char *file_name, void *p_addr, int offset, int len);
/**
 * Write contents to a file
 *
 * @param <int> i_directory_node_no
 * @param <const void *> p_addr contents to be written
 * @param <int> offset the offset to put the contents
 * @param <int> len bytes of the contents
 *
 * @return <int> > 0 (success) < 0 (error)
 *
 **/
extern int write_file (int i_directory_node_no, const void *p_addr, int offset, int len);
/**
 * Create a directory
 *
 * @param <char *> dir_name
 * 
 * @return <int> >= 0 (success), < 0 (error occured)
 *
 **/
extern int create_dir (char *dir_name);
/**
 * Remove a directory
 *
 * @param <const char *> dir_name
 *
 * @return <int> >= 0 (success), < 0 (error)
 *
 **/
extern int rm_dir (char *dir_name);
/**
 * remove a file
 *
 * @param <const char *> file_name
 *
 * @return <int> >= 0 (success), < 0 (error occured)
 *
 **/
extern int rm_file (const char *file_name);
/**
 *
 * Change current working directory to dir_name
 *
 * @param <const char *> dir_name target working directory
 *
 * @return <int> >= 0 (success), < 0 (error)
 *
 **/
extern int cd (const char *dir_name);
/**
 * List the next sibling tree node of current.
 *
 * @param <directory>
 *
 **/
extern int list_next_child (directory *p_dir);
/**
 *
 * Get current working directory
 *
 * @param <char *> p_addr cwd will be stored here
 *
 * @param <int> >= 0 (success), < 0 (error)
 *
 **/
extern int get_cwd (char *p_addr);
/**
 *
 * Get the remained space
 *
 * @return <int> >= 0 (success), < 0 (error)
 *
 **/
extern int get_free_bytes ();
//
#endif