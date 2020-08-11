#ifdef _WIN32
#ifdef _DEBUG
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p) _free_dbg(p, _NORMAL_BLOCK)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif /*_DEBUG*/
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hafs.h"

//#pragma comment(lib,"PXATFS.lib")
#pragma warning(disable : 4996)

HANDLE hConOut;
HANDLE hConIn;
unsigned char g_buf[8192];


#define TEST_COMMAND_MAX_LEN	5120
#define MAX_FILE_NAME			255

/* File access mode and open method flags (3rd argument of f_open) */
#define	FA_READ				0x01
#define	FA_WRITE			0x02
#define	FA_OPEN_EXISTING	0x00
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define	FA_OPEN_APPEND		0x30

// we can add __FUNCTION__, __LINE__, __FILE__ in these macros to output logs
#define TEST_ASSERT_EQ_EX(_rtn, _val, _err_num) \
    do {                                        \
        if (_rtn != _val) {                     \
            iRtn = _err_num;                    \
            goto END;                           \
        }                                       \
    } while (0)


// we can add __FUNCTION__, __LINE__, __FILE__ in these macros to output logs
#define TEST_ASSERT_NEQ_EX(_rtn, _val, _err_num) \
    do {                                         \
        if (_rtn == _val) {                      \
            iRtn = _err_num;                     \
            goto END;                            \
        }                                        \
    } while (0)

#define TEST_COPY_OUTDATA(pbOutBuffer, pnOutLen, pbInputBuffer, nInputLen, rtn_ok, rtn_buffer_small) \
    do {                                                                                             \
        if (!(pbOutBuffer)) {                                                                        \
            *(pnOutLen) = (nInputLen);                                                               \
            iRtn = (rtn_ok);                                                                         \
            goto END;                                                                                \
        }                                                                                            \
        if (*(pnOutLen) < (nInputLen)) {                                                             \
            *(pnOutLen) = (nInputLen);                                                               \
            iRtn = (rtn_buffer_small);                                                               \
            goto END;                                                                                \
        }                                                                                            \
        memcpy((pbOutBuffer), (pbInputBuffer), (nInputLen));                                         \
        *(pnOutLen) = (nInputLen);                                                                   \
    } while (0)

static int print_menuline(const char* const szContent) {
	int iRtn = -1;
	size_t i = 0, spaceCount1 = 0, spaceCount2 = 0;
	const size_t borderLen = 18, contentLen = 50;

	TEST_ASSERT_NEQ_EX(szContent, 0, -1);

	for (i = 0; i < borderLen; i++) {
		printf("=");
	}

	// spaceCount1 = (contentLen - strlen(szContent)) / 2;
	// spaceCount2 = (contentLen - strlen(szContent)) / 2 +
	//               ((strlen(szContent) % 2) ? (1) : (0));

	spaceCount1 = 8;
	spaceCount2 = (contentLen - strlen(szContent) - spaceCount1);

	for (i = 0; i < spaceCount1; i++) {
		printf(" ");
	}

	printf("%s", szContent);

	for (i = 0; i < spaceCount2; i++) {
		printf(" ");
	}

	for (i = 0; i < borderLen; i++) {
		printf("=");
	}
	printf("\n");

	iRtn = 0;
END:
	return iRtn;
}

int inputStringData(unsigned char* const pbStringData,
	size_t* const pnStringDataLen) {
	int iRtn = -1;

	char szStringData[TEST_COMMAND_MAX_LEN] = { 0 };

	TEST_ASSERT_NEQ_EX(pbStringData, 0, -1);
	TEST_ASSERT_NEQ_EX(pnStringDataLen, 0, -1);

	do {
#ifdef _WIN32
		gets_s(szStringData, sizeof(szStringData));
#else   //_WIN32
		fgets(szStringData, sizeof(szStringData), stdin);
		if (szStringData[strlen(szStringData) - 1] == '\n') {
			szStringData[strlen(szStringData) - 1] = '\0';
		}
#endif  //_WIN32

		if (strlen(szStringData) == 0) {
			printf("Length input invalid, please input again: ");
			continue;
		}

		break;
	} while (1);

	if (*pnStringDataLen < strlen(szStringData) + 1) {
		*pnStringDataLen = strlen(szStringData) + 1;
		iRtn = -1;
		goto END;
	}

	strcpy((char*)pbStringData, szStringData);
	*pnStringDataLen = strlen(szStringData) + 1;

	iRtn = 0;
END:
	return iRtn;
}

int test_format()
{
	PXATFS_FIND_DATA phfindfile;
	void* pfind;
	unsigned long rv;
	rv = PXATFS_Format(0, 0);
	return rv;
}

int test_get_size()
{
	unsigned long iRtn, rDataSize;

	iRtn = PXATFS_GetTotalSize(0, &rDataSize);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_GetFreeSize(0, &rDataSize);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

END:
	return iRtn;
}

int test_create_delete_dir()
{
	unsigned long iRtn;
	char byDirName[] = "dir1503";
	size_t iDirNameLen = sizeof(byDirName);

	iRtn = PXATFS_CreateDir(0, byDirName);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_DeleteDir(0, byDirName, PXATFS_DELETE_ALL_FILES);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

END:
	return iRtn;

}


int test_file_ops()
{
	void *pFile;
	unsigned long iRtn, iFileLen, iWriteLen, iReadLen;
	char byFileName[] = "file_rwtest", wData[262144], rData[262144];
	size_t iFileNameLen, iDataLen, i;

	iFileNameLen = sizeof(byFileName);

	iRtn = PXATFS_OpenFile(0, byFileName, FA_READ| FA_WRITE| FA_CREATE_NEW, &pFile);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_GetFileLength(pFile, &iFileLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	srand((unsigned int)time(0));
	for (i = 0; i < sizeof(wData); i++) {
		wData[i] = rand() % 0x100;
	}

	iDataLen = sizeof(wData);

	iRtn = PXATFS_WriteFile(pFile, wData, iDataLen, &iWriteLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_GetFileLength(pFile, &iFileLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_CloseFile(pFile);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_GetFileLength(pFile, &iFileLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_OpenFile(0, byFileName, FA_READ | FA_WRITE , &pFile);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	memset(rData, 0, sizeof(rData));
	iDataLen = sizeof(rData);
	iRtn = PXATFS_ReadFile(pFile, rData, iDataLen, &iReadLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	if ((memcmp(wData, rData, iReadLen) != 0) || (iWriteLen != iReadLen))
	{
		iRtn = 1;
		goto END;
	}

	iRtn = PXATFS_GetFileLength(pFile, &iFileLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_CloseFile(pFile);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_DeleteFile(0, byFileName);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);


END:
	return iRtn;

}


int test_file_ops2()
{
	void *pFile;
	unsigned long iRtn, iFileLen;
	char byFileName[] = "file_rwtest", byData[512],byNewFileName[] = "file_rwtest_2";
	size_t iFileNameLen, iDataLen, iWriteReadLen;

	iRtn = PXATFS_OpenFile(0, byFileName, FA_READ | FA_WRITE | FA_CREATE_NEW, &pFile);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_RenameFile(0, byFileName, byNewFileName);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_GetFileLength(pFile, &iFileLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_SetFilePos(pFile, 0, 10);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_GetFileLength(pFile, &iFileLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_Truncate(pFile, 100);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_GetFileLength(pFile, &iFileLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_CloseFile(pFile);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_DeleteFile(0, byNewFileName);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

END:
	return iRtn;

}

int test_dir_copy_total_path()
{
	void *pFile;
	int i, len;
	unsigned long iRtn, iWriteReadLen;
	char hafile1Path[] = "/HADir/DirSub1/DirSub2/file1.txt";
	unsigned char byData[10] = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a };
	char hostDir_1[256] = { 0 }, hostDir_2[256] = { 0 };

	iRtn = PXATFS_CreateDirEx(0, "/HADir");
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_CreateDirEx(0, "/HADir/DirSub1");
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_CreateDirEx(0, "/HADir/DirSub1/DirSub2");
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_OpenFile(0, hafile1Path, FA_READ | FA_WRITE | FA_CREATE_NEW, &pFile);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_WriteFile(pFile, byData, sizeof(byData), &iWriteReadLen);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_CloseFile(pFile);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

#ifdef _WIN32

	strcat(hostDir_1, "E:\HostDir");
	iRtn = PXATFS_CopyHADirToHost(0, "/HADir", hostDir_1);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_CopyHostDirToHA(0, hostDir_1, "/HADir2");
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	strcat(hostDir_2, "E:\HostDir2");
	iRtn = PXATFS_CopyHADirToHost(0, "/HADir2", hostDir_2);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

#else

	strcat(hostDir_1, "/HostDir");
	iRtn = PXATFS_CopyHADirToHost(0, "/HADir", hostDir_1);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	iRtn = PXATFS_CopyHostDirToHA(0, hostDir_1, "/HADir6");
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);

	strcat(hostDir_2, "/HostDir2");
	iRtn = PXATFS_CopyHADirToHost(0, "/HADir6", hostDir_2);
	TEST_ASSERT_EQ_EX(iRtn, 0, iRtn);


#endif

END:
	PXATFS_Format(0, 0);
	return iRtn;
}


#define FORMAT_AFTER_EACH_TEST 1

#define OUTPUT_LOOP_LOG 0

int test_performance()
{

	void *pFile;
	unsigned long iRtn, iFileLen, iWriteLen, iReadLen;
	long tickstart = 0, tickend = 0, milsecs = 0;
	char byFileName_1[] = "test2Mfile", byFileName_2[] = "test10Mfile", byFileName_3[] = "test50Mfile", byFileName_4[] = "test100Mfile";
	char *wrData;
	size_t iFileNameLen, i, j;
	size_t loopCount = 100;

	char szFileName[255];

	wrData = malloc(1024 * 1024 * 100);

	srand((unsigned int)time(0));
	for (i = 0; i < (1024 * 1024 * 100); i++) {
		wrData[i] = rand() % 0x100;
	}
	/*
#if FORMAT_AFTER_EACH_TEST
	PXATFS_Format(0, 0);
#endif
	//2M Write and read test
	milsecs = 0;
	for (size_t i = 0; i < loopCount; i++)
	{
#if OUTPUT_LOOP_LOG
		printf("Write 2M file test loop %ld starts\n", i);
#endif
		memset(szFileName, 0, sizeof(szFileName));
		sprintf(szFileName, "file_2m_%03ld", i);
		tickstart = CommUtil_GetTickCount();
		iRtn = PXATFS_OpenFile(0, szFileName, FA_WRITE | FA_CREATE_NEW, &pFile);
		if (iRtn != 0)
		{
			printf("create file failed in 2m write test, loop %ld\n", i);
			goto END;
		}
		iRtn = PXATFS_WriteFile(pFile, wrData, 1024 * 1024 * 2, &iWriteLen);
		if (iRtn != 0)
		{
			PXATFS_CloseFile(pFile);
			printf("write file failed in 2m write test, loop %ld\n", i);
			goto END;
		}
		else
		{
			PXATFS_CloseFile(pFile);
		}
		tickend = CommUtil_GetTickCount();
		milsecs += tickend - tickstart;
		printf("Write 2M file, loop count: %ld, cost time: %ldms\n", i, (tickend - tickstart));

	}


	printf("Write 2M file costs total %ldms, loop count: %ld, average: %ldms\n", milsecs, loopCount, milsecs / loopCount);

	milsecs = 0;
	for (size_t i = 0; i < loopCount; i++)
	{
#if OUTPUT_LOOP_LOG
		printf("Read 2M file test loop %ld starts\n", i);
#endif
		memset(szFileName, 0, sizeof(szFileName));
		sprintf(szFileName, "file_2m_%03ld", i);
		tickstart = CommUtil_GetTickCount();
		iRtn = PXATFS_OpenFile(0, szFileName, FA_READ, &pFile);
		if (iRtn != 0)
		{
			printf("open file failed in 2m read test, loop %ld\n", i);
			goto END;
		}
		iRtn = PXATFS_ReadFile(pFile, wrData, 1024 * 1024 * 2, &iReadLen);
		if (iRtn != 0)
		{
			PXATFS_CloseFile(pFile);
			printf("read file failed in 2m read test, loop %ld\n", i);
			goto END;
		}
		else
		{
			PXATFS_CloseFile(pFile);
		}
		tickend = CommUtil_GetTickCount();
		milsecs += tickend - tickstart;

		printf("Read 2M file, loop count: %ld, cost time: %ldms\n", i, (tickend - tickstart));
	}
	printf("Read 2M file costs total %ldms, loop count: %ld, average: %ldms\n", milsecs, loopCount, milsecs / loopCount);

#if FORMAT_AFTER_EACH_TEST
	PXATFS_Format(0, 0);
#endif*/

	//10M 
	milsecs = 0;
	for (size_t i = 0; i < loopCount; i++)
	{
#if OUTPUT_LOOP_LOG
		printf("Write 10M file test loop %ld starts\n", i);
#endif
		memset(szFileName, 0, sizeof(szFileName));
		sprintf(szFileName, "file_10m_%03ld", i);
		tickstart = CommUtil_GetTickCount();
		iRtn = PXATFS_OpenFile(0, szFileName, FA_WRITE | FA_CREATE_NEW, &pFile);
		if (iRtn != 0)
		{
			printf("create file failed in 10m write test, loop %ld\n", i);
			goto END;
		}
		iRtn = PXATFS_WriteFile(pFile, wrData, 1024 * 1024 * 10, &iWriteLen);
		if (iRtn != 0)
		{
			PXATFS_CloseFile(pFile);
			printf("write file failed in 10m write test, loop %ld\n", i);
			goto END;
		}
		else
		{
			PXATFS_CloseFile(pFile);
		}
		tickend = CommUtil_GetTickCount();
		milsecs += tickend - tickstart;

		printf("Write 10M file, loop count: %ld, cost time: %ldms\n", i, (tickend - tickstart));
	}
	printf("Write 10M file costs total %ldms, loop count: %ld, average: %ldms\n", milsecs, loopCount, milsecs / loopCount);

	milsecs = 0;
	for (size_t i = 0; i < loopCount; i++)
	{
#if OUTPUT_LOOP_LOG
		printf("Read 10M file test loop %ld starts\n", i);
#endif
		memset(szFileName, 0, sizeof(szFileName));
		sprintf(szFileName, "file_10m_%03ld", i);
		tickstart = CommUtil_GetTickCount();
		iRtn = PXATFS_OpenFile(0, szFileName, FA_READ, &pFile);
		if (iRtn != 0)
		{
			printf("open file failed in 10m read test, loop %ld\n", i);
			goto END;
		}
		iRtn = PXATFS_ReadFile(pFile, wrData, 1024 * 1024 * 10, &iReadLen);
		if (iRtn != 0)
		{
			PXATFS_CloseFile(pFile);
			printf("read file failed in 10m read test, loop %ld\n", i);
			goto END;
		}
		else
		{
			PXATFS_CloseFile(pFile);
		}
		tickend = CommUtil_GetTickCount();
		milsecs += tickend - tickstart;

		printf("Read 10M file, loop count: %ld, cost time: %ldms\n", i, (tickend - tickstart));
	}
	printf("Read 10M file costs total %ldms, loop count: %ld, average: %ldms\n", milsecs, loopCount, milsecs / loopCount);

#if FORMAT_AFTER_EACH_TEST
	PXATFS_Format(0, 0);
#endif

	//50M
	loopCount = 10;
	milsecs = 0;
	for (size_t i = 0; i < loopCount; i++)
	{
#if OUTPUT_LOOP_LOG
		printf("Write 50M file test loop %ld starts\n", i);
#endif
		memset(szFileName, 0, sizeof(szFileName));
		sprintf(szFileName, "file_50m_%03ld", i);
		tickstart = CommUtil_GetTickCount();
		iRtn = PXATFS_OpenFile(0, szFileName, FA_WRITE | FA_CREATE_NEW, &pFile);
		if (iRtn != 0)
		{
			printf("create file failed in 50m write test, loop %ld\n", i);
			goto END;
		}
		iRtn = PXATFS_WriteFile(pFile, wrData, 1024 * 1024 * 50, &iWriteLen);
		if (iRtn != 0)
		{
			PXATFS_CloseFile(pFile);
			printf("write file failed in 50m write test, loop %ld\n", i);
			goto END;
		}
		else
		{
			PXATFS_CloseFile(pFile);
		}
		tickend = CommUtil_GetTickCount();
		milsecs += tickend - tickstart;

		printf("Write 50M file, loop count: %ld, cost time: %ldms\n", i, (tickend - tickstart));
	}
	printf("Write 50M file costs total %ldms, loop count: %ld, average: %ldms\n", milsecs, loopCount, milsecs / loopCount);

	milsecs = 0;
	for (size_t i = 0; i < loopCount; i++)
	{
#if OUTPUT_LOOP_LOG
		printf("Read 50M file test loop %ld starts\n", i);
#endif
		memset(szFileName, 0, sizeof(szFileName));
		sprintf(szFileName, "file_50m_%03ld", i);
		tickstart = CommUtil_GetTickCount();
		iRtn = PXATFS_OpenFile(0, szFileName, FA_READ, &pFile);
		if (iRtn != 0)
		{
			printf("open file failed in 50m read test, loop %ld\n", i);
			goto END;
		}
		iRtn = PXATFS_ReadFile(pFile, wrData, 1024 * 1024 * 50, &iReadLen);
		if (iRtn != 0)
		{
			PXATFS_CloseFile(pFile);
			printf("read file failed in 10m read test, loop %ld\n", i);
			goto END;
		}
		else
		{
			PXATFS_CloseFile(pFile);
		}
		tickend = CommUtil_GetTickCount();
		milsecs += tickend - tickstart;

		printf("Read 50M file, loop count: %ld, cost time: %ldms\n", i, (tickend - tickstart));
	}
	printf("Read 50M file costs total %ldms, loop count: %ld,average: %ldms\n", milsecs, loopCount, milsecs / loopCount);

#if FORMAT_AFTER_EACH_TEST
	PXATFS_Format(0, 0);
#endif

	//100M 
	milsecs = 0;
	for (size_t i = 0; i < loopCount; i++)
	{
#if OUTPUT_LOOP_LOG
		printf("Write 100M file test loop %ld starts\n", i);
#endif
		memset(szFileName, 0, sizeof(szFileName));
		sprintf(szFileName, "file_100m_%03ld", i);
		tickstart = CommUtil_GetTickCount();
		iRtn = PXATFS_OpenFile(0, szFileName, FA_WRITE | FA_CREATE_NEW, &pFile);
		if (iRtn != 0)
		{
			printf("create file failed in 100m write test, loop %ld\n", i);
			goto END;
		}
		iRtn = PXATFS_WriteFile(pFile, wrData, 1024 * 1024 * 100, &iWriteLen);
		if (iRtn != 0)
		{
			PXATFS_CloseFile(pFile);
			printf("write file failed in 100m write test, loop %ld\n", i);
			goto END;
		}
		else
		{
			PXATFS_CloseFile(pFile);
		}
		tickend = CommUtil_GetTickCount();
		milsecs += tickend - tickstart;

		printf("Write 100M file, loop count: %ld, cost time: %ldms\n", i, (tickend - tickstart));
	}
	printf("Write 100M file costs total %ldms, loop count: %ld, average: %ldms\n", milsecs, loopCount, milsecs / loopCount);

	milsecs = 0;
	for (size_t i = 0; i < loopCount; i++)
	{
#if OUTPUT_LOOP_LOG
		printf("Read 100M file test loop %ld starts\n", i);
#endif
		memset(szFileName, 0, sizeof(szFileName));
		sprintf(szFileName, "file_100m_%03ld", i);
		tickstart = CommUtil_GetTickCount();
		iRtn = PXATFS_OpenFile(0, szFileName, FA_READ, &pFile);
		if (iRtn != 0)
		{
			printf("open file failed in 100m read test, loop %ld\n", i);
			goto END;
		}
		iRtn = PXATFS_ReadFile(pFile, wrData, 1024 * 1024 * 100, &iReadLen);
		if (iRtn != 0)
		{
			PXATFS_CloseFile(pFile);
			printf("read file failed in 100m read test, loop %ld\n", i);
			goto END;
		}
		else
		{
			PXATFS_CloseFile(pFile);
		}
		tickend = CommUtil_GetTickCount();
		milsecs += tickend - tickstart;

		printf("Read 100M file, loop count: %ld, cost time: %ldms\n", i, (tickend - tickstart));
	}
	printf("Read 100M file costs total %ldms, loop count: %ld, average: %ldms\n", milsecs, loopCount, milsecs / loopCount);

#if FORMAT_AFTER_EACH_TEST
	PXATFS_Format(0, 0);
#endif

END:
	free(wrData);
	return iRtn;
}


unsigned long get_sn()
{
	char sn[20];
	unsigned long rv, snlen = sizeof(sn);

	rv = PXATFS_GetDevSN(0, sn, &snlen);

	printf("Get device sn: %s\n", sn);
	return rv;

}

unsigned long getallfiles()
{
	unsigned long rv;
	int pathoffset;
	PXATFS_FIND_DATA data;
	char path[256] = { 0 },findfilename[256] = { 0 };
	void *find = NULL;

	pathoffset = 1;
	strcpy(path, "/*");
	rv = PXATFS_FindFirstFile(0, path, &find, &data);
	while (rv == 0)
	{
		memset(findfilename, 0, sizeof(findfilename));
		strcpy(findfilename, data.fileName);
		if (data.isDir)
		{
			printf("Find Directory: %s\n", findfilename);
			memcpy(path + pathoffset, findfilename, strlen(findfilename));			
			strcat(path, "/*");
			pathoffset += strlen(findfilename) + 1;
			rv = PXATFS_FindFirstFile(0, path, &find, &data);
			continue;
		}
		else
		{
			printf("Find File: %s\n", findfilename);
		}

		if (!find)
		{
			break;
		}

		memset(&data, 0, sizeof(data));
		rv = PXATFS_FindNextFile(find, &data);

	}

	if (find)
	{
		PXATFS_FindClose(find);
	}

	rv = 0;
	return rv;

}



int main()
{
#if defined(_WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_CRT_DF);
#endif

	unsigned long iRtn = -1;
	unsigned char nLoop = 0;
	int iSelect = 0;

	nLoop = 1;
	do 
	{
		printf("===========================================================\n");
		print_menuline("0. exit");
		print_menuline("1. format device");
		print_menuline("2. get size");
		print_menuline("3. create delete dir");
		print_menuline("4. file write read");
		print_menuline("5. file change");
		print_menuline("6. directory copy whole path");
		print_menuline("7. test performance");
		print_menuline("8. get sn");
		print_menuline("9. find files");
		printf("Please select case: ");
#ifdef _WIN32
		scanf_s("%d", &iSelect);
#else
		scanf("%d", &iSelect);
#endif
		getchar();

		switch (iSelect)
		{
			case 0:
				nLoop = 1;
				break;
			case 1:
				iRtn = test_format();
				printf("format return:%d\n", iRtn);
				break;
			case 2:
				iRtn = test_get_size();
				printf("get size return:%d\n", iRtn);
				break;
			case 3:
				iRtn = test_create_delete_dir();
				printf("create delete dir return:%d\n", iRtn);
				break;
			case 4:
				iRtn = test_file_ops();
				printf("file test return:%d\n", iRtn);
				break;
			case 5:
				iRtn = test_file_ops2();
				printf("file test 2 return:%d\n", iRtn);
				break;
			case 6:
				iRtn = test_dir_copy_total_path();
				printf("dir copy with totalpath return:%d\n", iRtn);
				break;
			case 7:
				iRtn = test_performance();
				printf("test performance return:%d\n", iRtn);
				break;
			case 8:
				iRtn = get_sn();
				printf("get sn return:%d\n", iRtn);
				break;
			case 9:
				iRtn = getallfiles();
				printf("get all files return:%d\n", iRtn);
				break;
			default:
				printf("Input invalid, please input again\n");
				break;
		}

	} while (nLoop);

	getchar();
	iRtn = 0;
	return 0;

}