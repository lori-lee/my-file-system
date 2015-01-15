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
#define E_WT            -0x19  //error while writing disk
#define E_RD            -0x1a  //error while reading disk

#define E_INVALID_PARAM -0x20  //invalid parameter

const char *str_error (int errno)
{
    switch (errno) {
        case E_NO_FS:           return "file system not found or destroied";
        case E_FS_CHCK:         return "file system checksum error";
        case E_LACK_SPACE:      return "space not enough";
        case E_DISK_VALUE:      return "disk content error/corrupted";
        case E_NO_DIR:          return "directory not exist";
        case E_NO_FILE:         return "file not exist";
        case E_DIR_EXIST:       return "directory already exists";
        case E_FILe_EXIST:      return "file already exists";
        case E_DIR_NOT_EMPTY:   return "target directory is not empty";
        case E_WT:              return "error while writing disk";
        case E_RD:              return "error while reading disk";
        case E_INVALID_PARAM:   return "invalid parameter";
        default:                return "unknown";
    }
}
#endif
