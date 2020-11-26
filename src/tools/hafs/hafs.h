#pragma once

#ifndef _HAFS_H
#define _HAFS_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
#define IN
#define OUT
#define HANDLE void*
#define INVALID_HANDLE_VALUE ((void*)-1)
#define HAFS_EXPORT __attribute__((visibility("default")))
#else
#include <windows.h>
#define HAFS_EXPORT
#endif  //_WIN32

#include <stdint.h>
#include <stdio.h>

//#define _PXATFS_RECORD_LOG

#define LOGICAL_DRIVE_INDEX 0  //default volume index

#define STR_DEFAULT_ENTRY_PATH "0:/"  // default path of entry

//  待客户提供
#define PXATFS_DELETE_DIR_ONLY 0
#define PXATFS_DELETE_ALL_FILES 1

#define PXATFS_CLUSTER_SIZE 32768

#define MAX_FILENAME_LENGTH 128

typedef struct
{
    unsigned char isDir;
    char fileName[MAX_FILENAME_LENGTH];
    unsigned long fileSize;

} PXATFS_FIND_DATA;

//1.获取设备序列号
// Get SN.
HAFS_EXPORT unsigned long PXATFS_GetDevSN(
    IN int hDevice,
    OUT char* pszDevSN,
    OUT unsigned long* ulDevSNLen);

//2.格式化设备
// Format the hidden area.
HAFS_EXPORT unsigned long PXATFS_Format(
    IN int hDevice,
    IN unsigned char flag);

//3.获取设备总大小
// Retrieves the total size of the hidden area.
HAFS_EXPORT unsigned long PXATFS_GetTotalSize(
    IN int hDevice,
    OUT unsigned long* pulTotalSize);

//4.获取设备剩余空间
HAFS_EXPORT unsigned long PXATFS_GetFreeSize(
    IN int hDevice,
    OUT unsigned long* pulFreeSize);

//5.打开文件
// Open the special file.
HAFS_EXPORT unsigned long PXATFS_OpenFile(
    IN int hDevice,
    IN const char* const pszFileName,
    IN const unsigned long ulFlags,
    OUT void** phFile);

//6.关闭文件
// Close file.
HAFS_EXPORT unsigned long PXATFS_CloseFile(
    IN void* hFile);

//7.设置文件读写文件位置
// This function moves the file pointer of an open file.
HAFS_EXPORT unsigned long PXATFS_SetFilePos(
    IN void* hFile,
    IN const unsigned long ulMode,
    IN int nOffset);

//8.修改文件大小
// 修改隐藏区文件大小
HAFS_EXPORT unsigned long PXATFS_Truncate(
    IN void* hFile,
    IN const unsigned long ulLength);
//9.写文件
// Write data from the current offset of the file.
HAFS_EXPORT unsigned long PXATFS_WriteFile(
    IN void* hFile,
    IN const unsigned char* const pbData,
    IN unsigned long ulDataLen,
    OUT unsigned long* pulWriteLen);

//10.读文件

// Read data from the current offset of the file.
// Parameters:
// IN void *hFile: Supplies the handle of the special file.
// OUT unsigned char *pbData: Receives the read data, suppose the buffer is big enough.
// IN unsigned long ulDataLen: Supplies the length to be read.
// OUT unsigned long *pulReadLen: Receives the length in bytes read from the device in fact.
HAFS_EXPORT unsigned long PXATFS_ReadFile(
    IN void* hFile,
    OUT const unsigned char* const pbData,
    IN unsigned long ulDataLen,
    OUT unsigned long* pulReadLen);

//11.获取文件大小
// Get the file length.
// Parameters:
// IN void *hFile: Supplies the handle of the special file.
// OUT unsigned long *pulFileLen: Receives the file length.
HAFS_EXPORT unsigned long PXATFS_GetFileLength(
    IN void* hFile,
    OUT unsigned long* pulFileLen);

//12.重命名文件
// Rename the special file.
// Parameters:
// IN int hDevice: Supplies the device handle.
// IN unsigned short *pszOldName: Supplies the old file name.
// IN unsigned short *pszNewName: Supplies the new file name.
HAFS_EXPORT unsigned long PXATFS_RenameFile(
    IN int hDevice,
    IN const char* const pszOldName,
    IN const char* const pszNewName);
// IN unsigned short *pszNewName: Supplies the new file name.
//13.删除文件
// Delete file.
// Parameters:
// IN int hDevice: Supplies the device handle.
// IN unsigned short *pszFileName: Pointer to the file name to be deleted.
HAFS_EXPORT unsigned long PXATFS_DeleteFile(
    IN int hDevice,
    IN const char* const pszFileName);

//14.创建目录
// Create directory.
// Parameters:
// IN int hDevice: Supplies the device handle.
// IN unsigned short *pszDirName: Supplies the directory name.
HAFS_EXPORT unsigned long PXATFS_CreateDir(
    IN int hDevice,
    IN const char* const pszDirName);

//15.创建多层目录
HAFS_EXPORT unsigned long PXATFS_CreateDirEx(
    IN int hDevice,
    IN const char* const pszDirName);

//16.删除目录，含非空目录
// Delete the directory.
// If pszDirName is "\\", it means the root directory.
// When deleting "\\", HAFS_CANT_DELETE_ROOT_ERROR will be returned.
// Parameters:
// IN int hDevice: Supplies the device handle.
// IN unsigned short *pszDirName: Supplies the directory to be deleted.
// IN unsigned long ulFlag: The mode of deleting directory. It can be one of the following.
//						HAFS_DELETE_DIR_ONLY: Just delete the directory only.
//						HAFS_DELETE_ALL_FILES: Delete the directory and all the files(sub-directories) in the directory.
HAFS_EXPORT unsigned long PXATFS_DeleteDir(
    IN int hDevice,
    IN const char* const pszDirName,
    IN const unsigned long ulFlag);

//17.获取当前目录下的第一个文件或目录
// Find the first file in the current directory. Can be "*".
// If the function fails, phFind will return INVALID_HANDLE_VALUE.
// Parameters:
// IN int hDevice: Supplies the device handle.
// IN unsigned short *pszFileName: Supplies the file name.
// OUT void *phFind: Receives the find handle.
// OUT HAFS_FIND_DATA *pSDHAFindData: Receives HAFS_FIND_DATA of the find file.

HAFS_EXPORT unsigned long PXATFS_FindFirstFile(
    IN int hDevice,
    IN const char* const pszFileName,
    OUT void** phFind,
    OUT PXATFS_FIND_DATA* pSDHAFindData);

//18.获取当前目录下的下一个文件或目录
// Find the next file.
// Parameters:
// IN void *hFind: Supplies the find handle.
// IN OUT HAFS_FIND_DATA *pSDHAFindData: Supplies the current file, receives HAFS_FIND_DATA of the next file.
HAFS_EXPORT unsigned long PXATFS_FindNextFile(
    IN void* hFind,
    IN PXATFS_FIND_DATA* pSDHAFindData);

//19.关闭查找句柄
// Find close.
// Parameters:
// IN void *hFind: Supplies the find handle.
HAFS_EXPORT unsigned long PXATFS_FindClose(
    IN void* hFind);

//20.拷贝文件到本地
// Copy file from the hidden area to the host.
// Parameters:
// IN int hDevice: Supplies the device handle.
// IN unsigned short *pszSrcFileName：Supplies the source file name.
// IN char *pszDestFileName: Supplies the destination file name.
HAFS_EXPORT unsigned long PXATFS_CopyHAFileToHost(
    IN int hDevice,
    IN const char* const pszSrcFileName,
    IN const char* const pszDestFileName);

//22.拷贝文件到设备
// Copy file from the host to the hidden area.
// Parameters:
// IN int hDevice: Supplies the device handle.
// IN char *pszSrcFileName: Supplies the source file name.
// IN unsigned short *pszDestFileName: Supplies the destination file name.
HAFS_EXPORT unsigned long PXATFS_CopyHostFileToHA(
    IN int hDevice,
    IN const char* const pszSrcFileName,
    IN const char* const pszDestFileName);

//23.拷贝目录到设备

// Copy all the files in the directory from the host to the hidden area. The destination directory name must be existed already.
// Parameters:
// IN int hDevice: Supplies the device handle.
// IN char *pszSrcDirName: Supplies the source directory name.
// IN unsigned short *pszDestDirName: Supplies the destination directory name. The destination directory name must be existed already.
HAFS_EXPORT unsigned long PXATFS_CopyHostDirToHA(
    IN int hDevice,
    IN const char* const pszSrcDirName,
    IN const char* const pszDestDirName);

//24.拷贝目录到本地
// Copy all the files in the directory from the hidden area to the host. The destination directory name must be existed already.
// Parameters:
// IN int hDevice: Supplies the device handle.
// IN unsigned short *pszSrcDirName: Supplies the source directory name.
// IN char *pszDestDirName: Supplies the destination directory name. The destination directory name must be existed already.
HAFS_EXPORT unsigned long PXATFS_CopyHADirToHost(
    IN int hDevice,
    IN const char* const pszSrcDirName,
    IN const char* const pszDestDirName);
//25.取文件信息
HAFS_EXPORT unsigned long PXATFS_GetFileInfo(
    IN int hDevice,
    IN const char* const pszFileName,
    OUT PXATFS_FIND_DATA* pFindData);

//26.取设备是否存在 0 - exist 1 - not exist
HAFS_EXPORT unsigned long PXATFS_IsDiskExist(
    IN int hDevice);

//27.check file md5
HAFS_EXPORT unsigned long PXATFS_CheckFileMD5(
    IN int hDevice,
    IN const char* const pszFileName,
    IN const char* const pszMD5Str);

#ifdef __cplusplus
}
#endif
#endif