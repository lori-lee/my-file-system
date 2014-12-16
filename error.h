#ifndef _MY_ERROR_H_
#define _MY_ERROR_H_

#define E_NO_FS -0x10          //file system not found or destroied
#define E_FS_CHCK -0x11        //file system checksum error
#define E_LACK_SPACE -0x12     //space not enough
#define E_DISK_VALUE -0x13     //disk content error
#define E_NO_DIR     -0x14     //directory not exist
#define E_NO_FILE    -0x15     //file not exist
#define E_DIR_EXIST  -0x16     //directory already exists
#define E_FILe_EXIST -0x17     //file already exists
#define E_DIR_NOT_EMPTY -0x18  //target directory is not empty

#define E_INVALID_PARAM -0x20  //invalid parameter
#endif
