#include "error.h"
const char *str_error (int32 errno)
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
