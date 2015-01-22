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
    char checksum;//0x0 the xor value of the super block
    uchar version;//0x1
    uchar byte_order;//0x2, 0 -- little endian, 1 -- big endian
    uchar page_size;//0x3 actual page size = 2 ^ pageSize (>= 256 = 2 ^ 8)

    char magic[4];//0x4 -- 0x4d 79 46 53 (MyFS)
    int32 total_pages;//0x8 blocks contained of the disk
    int32 create_datetime;//0xc the creation datetime of MyFS, year[6bit].month[4bit].day[5bit].hour[5bit].minute[6bit].sec[6bit]
    // 
    int32 pages_alloc_table;//0x10 the pages occupied by allocation table
    int32 first_idle_page;//0x14 the first unused page no
    int32 last_idle_page;//0x18 the last unused page no
    int32 idle_pages_num;//0x1c the total number of unused pages
    int32 directory;//0x20 the first page NO. of directory
    int32 directory_last;//0x24 the last page NO. of direcotry
    int32 PAT_start[PAT_PRE_INT_NUM];
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
    int32 parent_filesize;//0xc node index, for file node, this field record the file length
    int32 sibling;//0x10 node index
    int32 first_child_lastpage;//0x14 node index
    int32 first_data_page;//0x18 first page no of the data stored, only file should set it
    int32 create_datetime;//0x1c the creation datetime of MyFS, year[6bit].month[4bit].day[5bit].hour[5bit].minute[6bit].sec[6bit]
}directory;

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

static directory *g_p_current_dir = NULL;
static int32 g_i_current_dir_index = PAGE_NULL;
#define DIR_NODE_NUM_PER_PAGE  (PAGE_SIZE >> 3)
#define I_NAME_LEN 8
#define I_EXT_LEN  3

//functions declaration
/**
 * return the type of system order byte
 *
 * @return <enum> LITTLE_ENDIAN | BIG_ENDIAN
 **/
extern int32 _little_big_endian ();
/**
 * write data to the page specified by pageNo
 *
 * @param <int32> page_no  the page NO to be written
 * @param <const char*> p_src the page content to be written
 * @param <int32> len the content length
 *
 * @return <int32> the length written (>=0) or error occurred (< 0)
 *
 **/
extern int32 _write_page (int32 page_no, const void *p_src, int32 len);
/**
 * Write contents to the disk
 * @param <int32> page_no the page NO to be written
 * @param <int32> offset the page offset to be written
 * @param <const void *> p_src the start address of the contents
 * @param <int32> len the length of the content
 *
 * @return <int32> (>=0) the length written, or error (< 0)
 **/
extern int32 _write_page_offset (int32 page_no, int32 offset, const void *p_src, int32 len);
/**
 * Write the super page's content
 * @param <super_page *> p_super_page
 *
 * @return <int32> the length written (>=0) or error occurred (< 0)
 **/
extern int32 _write_super_page (super_page *p_super_page);
/**
 * read the data of a page
 *
 * @param <int32>  page_no  the page index (based on 0) to be read
 * @param <void *> p_page_buffer the buffer to store the page content
 *
 * @return <int32> the length read (>=0) or error occurred (< 0)
 **/
extern void *_read_page (int32 page_no, void *p_page_buffer);
/**
 * Read the content specified by page_no and page offset
 * 
 * @param <int32> page_no  the page to be read
 * @param <int32> offset   the offset(based on 0) relative the beginning of the page
 * @param <void *>p_buffer the buffer to store the content
 * @param <int32> len  the length to be read
 * 
 * @return <int32> the length read(>=0) or an error occured (< 0)
 *
 **/
extern void *_read_page_offset (int32 page_no, int32 offset, void *p_buffer, int32 len);
/**
 *
 * Convert the byte order of the content of a directory node
 * @param <directory *> p_dir
 * @param <enum> from
 * @param <enum> to
 *
 **/
extern void *_convert_directory_byte_order (directory *p_dir, int32 from, int32 to);
/**
 * convert the byte order of super page's content
 * @param <super_page *>  p_super_page the start address of the content
 * @param <enum> from (LITTLE_ENDIAN | BIG_ENDIAN)
 * @param <enum> to (LITTLE_ENDIAN | BIG_ENDIAN)
 *
 * @return <void *> p_super_page
 **/
extern void *_convert_super_page_byte_order (super_page *p_super_page, int32 from, int32 to);
/**
 * convert the byte order of the content pointed by p_page_addr
 * @param <void *>  p_page_addr 
 * @param <enum> from (LITTLE_ENDIAN | BIG_ENDIAN)
 * @param <enum> to (LITTLE_ENDIAN | BIG_ENDIAN)
 *
 * @return <void *> p_page_addr
 **/
extern void *_convert_page_byte_order (void *p_page_addr, int32 from, int32 to);
/**
 * calculate the PAT offset of page, return page, and *pageIndex will be the offset in the related page.
 * 
 * @param <int32> page_no (based on 0)
 *
 * @return <int32> page no & page offset will be store back to page_no
 **/		
extern int32 _calc_page_offset_in_PAT (int32 *page_no);
/**
 * calculate the pages should be occupied by page allocation table
 *
 * @return <int32>
 **/
static int32 _calc_PAT_pages ();
/**
 * initialize a link list, will set *p_start = start, *(p_start + 1) = start + 1, ..., *(p_start + count - 1) = start + count - 1
 *
 * @param <int32 *> p_start
 * @param <int32> start
 * @param <int32> count
 *
 * @return <int32> count if success, error (< 0)
 **/
static int32 _init_page_link_list (int32 *p_start, int32 start, int32 count);
/**
 * initialize the super page
 *
 **/
static int32 _init_super_page ();
/**
 * calcuate the checksum start from p_mem
 * 
 * @param <const char *> p_mem
 * @param <int32> len the bytes to be calculated
 *
 * @return <char>
 **/
static char _checksum (const char *p_mem, int32 len);
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
 * @return <int32> the last page NO of PAT
 *
 **/
static int32 _calc_PAT_page_end ();
/**
 *
 * Calcuate the last page NO of directory
 *
 * @return <int32>
 **/
static int32 _calc_dir_page_end ();
/**
 * Create root directory
 *
 **/
static int32 _create_root_dir ();
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
 * @return <int32> > 0 (success), <= 0 (error)
 *
 **/
extern int32 is_FS_exist ();
/**
 * Get the parent directory by path name
 * 
 * @param <char *> p_dir_name
 * @param <directory *> p_parent_directory parent directory node value will be stored here
 *
 * @return <int32> the directory index of parent directory
 *
 **/
static int32 _get_parent_directory_value_by_path_name (char *p_dir_name, directory *p_parent_directory);
static int32 _get_directory_no_by_name (char *p_dir_name);
/**
 *
 * Open a file
 *
 * @param <char *> file_name
 * @param <int32> create_if_not_exist (if this value is not 0, then a new empty file will be created if it does not exist before)
 *
 * @return <int32> > 0 (the directory node index of the file), < 0 (error)
 *
 **/
extern int32 open_file (char *file_name, int32 create_if_not_exist);
/**
 * create a file
 *
 * @param <const char *> file_name (e.g. '/path/path/file.ext', 'file.ext')
 * @param <const void *> p_file_data contents of the file
 * @param <int32> len length of content's bytes
 *
 * @return <int32> > 0 (success), < 0 (error occured)
 *
 **/
extern int32 create_file (const char *file_name, const void *p_file_data, int32 len);
/**
 * read a file's content into a buffer
 *
 * @param <const char *> file_name 
 * @param <void *> p_addr buffer the file content
 * @param <int32> offset the offset of file to be read
 * @param <int32> len bytes to be read
 *
 * @return <int32> > 0 (success, may less than len if no more content exists), 0 -- already the file end, < 0 (error)
 *
 **/
extern int32 read_file (const char *file_name, void *p_addr, int32 offset, int32 len);
/**
 * Write contents to a file
 *
 * @param <int32> i_directory_node_no
 * @param <const void *> p_addr contents to be written
 * @param <int32> offset the offset to put the contents
 * @param <int32> len bytes of the contents
 *
 * @return <int32> > 0 (success) < 0 (error)
 *
 **/
extern int32 write_file (int32 i_directory_node_no, const void *p_addr, int32 offset, int32 len);
/**
 * Create a directory
 *
 * @param <char *> dir_name
 * 
 * @return <int32> >= 0 (success), < 0 (error occured)
 *
 **/
extern int32 create_dir (char *dir_name);
/**
 * Remove a directory
 *
 * @param <const char *> dir_name
 *
 * @return <int32> >= 0 (success), < 0 (error)
 *
 **/
extern int32 rm_dir (char *dir_name);
/**
 * remove a file
 *
 * @param <const char *> file_name
 *
 * @return <int32> >= 0 (success), < 0 (error occured)
 *
 **/
extern int32 rm_file (char *file_name);
/**
 *
 * Change current working directory to dir_name
 *
 * @param <const char *> dir_name target working directory
 *
 * @return <int32> >= 0 (success), < 0 (error)
 *
 **/
extern int32 cd (char *dir_name);
/**
 * List the next sibling tree node of current.
 *
 * @param <directory>
 *
 **/
extern int32 list_next_child (char *p_dir_name, char **p_buff);
/**
 *
 * Get current working directory
 *
 * @param <char *> p_addr cwd will be stored here
 *
 * @param <int32> >= 0 (success), < 0 (error)
 *
 **/
extern int32 get_cwd (char *p_addr);
/**
 *
 * Get the remained space
 *
 * @return <int32> >= 0 (success), < 0 (error)
 *
 **/
extern int32 get_free_bytes ();
//
#endif
