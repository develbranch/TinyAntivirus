#include <gtest/gtest.h>
#include <TinyAvCore.h>
#include <string.h>
#include "../TinyAvCore/FileSystem/FileFsStream.h"

extern WCHAR szTestcase[MAX_PATH];

TEST(FileFsStream, Seek)
{
	HANDLE hFile = CreateFileW(szTestcase, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(INVALID_HANDLE_VALUE, hFile);
	IFsStream * fsStream;
	ULARGE_INTEGER pos;
	fsStream = new CFileFsStream();
	fsStream->SetFileHandle((void*)hFile);
	LARGE_INTEGER offset = {};
	offset.QuadPart = 200;
	ASSERT_HRESULT_SUCCEEDED(fsStream->Seek(&pos, offset, IFsStream::FsStreamBegin));
	ASSERT_EQ(200, pos.QuadPart);
	offset.QuadPart = 200;
	ASSERT_HRESULT_SUCCEEDED(fsStream->Seek(&pos, offset, IFsStream::FsStreamCurrent));
	ASSERT_EQ(400, pos.QuadPart);
	offset.LowPart = -100;
	offset.HighPart = -1;
	ASSERT_HRESULT_SUCCEEDED(fsStream->Seek(&pos, offset, IFsStream::FsStreamCurrent));
	ASSERT_EQ(300, pos.QuadPart);
	ASSERT_HRESULT_SUCCEEDED(fsStream->Tell(&pos));
	ASSERT_EQ(300, pos.QuadPart);
	offset.LowPart = -1;
	offset.HighPart = -1;
	ASSERT_HRESULT_SUCCEEDED(fsStream->Seek(&pos, offset, IFsStream::FsStreamEnd));
	ASSERT_EQ(0x100000 + offset.QuadPart, pos.QuadPart);
	CloseHandle(hFile);
	fsStream->Release();
}

TEST(FileFsStream, Tell)
{
	HANDLE hFile = CreateFileW(szTestcase, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(INVALID_HANDLE_VALUE, hFile);
	IFsStream * fsStream;
	fsStream = new CFileFsStream();
	fsStream->SetFileHandle((void*)hFile);
	LARGE_INTEGER distanceToMove = {};
	distanceToMove.QuadPart = 200;
	ULARGE_INTEGER pos;
	ASSERT_HRESULT_SUCCEEDED(fsStream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin));
	ASSERT_EQ(200, pos.QuadPart);
	ASSERT_HRESULT_SUCCEEDED(fsStream->Tell(&pos));
	ASSERT_EQ(200, pos.QuadPart);
	ASSERT_HRESULT_SUCCEEDED(fsStream->Seek(&pos, distanceToMove, IFsStream::FsStreamCurrent));
	ASSERT_EQ(400, pos.QuadPart);
	distanceToMove.QuadPart = -100;
	ASSERT_HRESULT_SUCCEEDED(fsStream->Seek(&pos, distanceToMove, IFsStream::FsStreamCurrent));
	ASSERT_EQ(300, pos.QuadPart);
	ASSERT_HRESULT_SUCCEEDED(fsStream->Seek(&pos, distanceToMove, IFsStream::FsStreamEnd));
	ASSERT_EQ(0x100000 + distanceToMove.QuadPart, pos.QuadPart);
	CloseHandle(hFile);
	fsStream->Release();
}

TEST(FileFsStream, ReadAt)
{
	HANDLE hFile = CreateFileW(szTestcase, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(INVALID_HANDLE_VALUE, hFile);
	IFsStream * fsStream;
	fsStream = new CFileFsStream();
	fsStream->SetFileHandle((void*)hFile);
	
	char buffer[0x400];
	ULONG readSize;
	
	unsigned char testcase1[0x40] = {
		0x8D, 0x5C, 0x8D, 0xEB, 0x5D, 0xDE, 0x4B, 0xA6, 0xE1, 0x9E, 0xC3, 0x82, 0xB2, 0x93, 0x17, 0x55,
		0xA1, 0xC6, 0x57, 0xD1, 0xC3, 0x57, 0x14, 0x81, 0x5F, 0x27, 0x2D, 0xDB, 0xEF, 0x1E, 0xD4, 0xDE,
		0xA3, 0x38, 0x77, 0x3F, 0x44, 0x85, 0xB5, 0x35, 0x14, 0x8E, 0x71, 0x2B, 0x01, 0x50, 0x6F, 0xBD,
		0x7C, 0xD8, 0xB7, 0x12, 0x03, 0x43, 0xC7, 0x35, 0x47, 0x0B, 0x8B, 0x35, 0xB1, 0x70, 0xB6, 0xC2
	};

	LARGE_INTEGER offset = {};
	
	// read data in cache
	offset.QuadPart = 0;
	ASSERT_HRESULT_SUCCEEDED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, buffer, sizeof(testcase1), &readSize));
	ASSERT_EQ(sizeof(testcase1), readSize);
	ASSERT_TRUE(0 == memcmp(buffer, testcase1, sizeof(testcase1)));

	unsigned char testcase2[0x40] = {
		0x77, 0x0A, 0xC8, 0x31, 0x90, 0x5E, 0xBB, 0x5D, 0x67, 0x14, 0x91, 0x40, 0x63, 0xFE, 0x10, 0x4F,
		0xEC, 0x16, 0xDF, 0x1F, 0xBE, 0x98, 0x84, 0x3E, 0xB8, 0xB5, 0x16, 0x68, 0xEA, 0x14, 0xF8, 0x7E,
		0x96, 0x17, 0x3C, 0x82, 0x86, 0x56, 0x80, 0xD2, 0x04, 0x2B, 0xCA, 0xEB, 0x84, 0xD1, 0x04, 0xB4,
		0x93, 0xF4, 0x5F, 0xAF, 0xC1, 0xA2, 0x14, 0xDF, 0x85, 0x8A, 0x79, 0xDB, 0x47, 0x53, 0x73, 0x52
	};

	// read data without cache + update cache
	offset.QuadPart = DEFAULT_MAX_CACHE_SIZE +0x10;
	ASSERT_HRESULT_SUCCEEDED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, buffer, sizeof(testcase2), &readSize));
	ASSERT_EQ(sizeof(testcase2), readSize);
	ASSERT_TRUE(0 == memcmp(buffer, testcase2, sizeof(testcase2)));

	// cache hit!
	offset.QuadPart = DEFAULT_MAX_CACHE_SIZE + 0x20;
	ASSERT_HRESULT_SUCCEEDED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, buffer, 0x20, &readSize));
	ASSERT_EQ(0x20, readSize);
	ASSERT_TRUE(0 == memcmp(buffer, testcase2 + 0x10, 0x20));

	// cache not found
	offset.QuadPart = DEFAULT_MAX_CACHE_SIZE + 0x10;
	ASSERT_HRESULT_SUCCEEDED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, buffer, 0x60, &readSize));
	ASSERT_EQ(0x60, readSize);
	ASSERT_TRUE(0 == memcmp(buffer, testcase2, sizeof(testcase2)));

	// cache not found, re-test testcase1
	offset.QuadPart = 0;
	ASSERT_HRESULT_SUCCEEDED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, buffer, sizeof(testcase1), &readSize));
	ASSERT_EQ(sizeof(testcase1), readSize);
	ASSERT_TRUE(0 == memcmp(buffer, testcase1, sizeof(testcase1)));

	CloseHandle(hFile);
	fsStream->Release();
}

TEST(FileFsStream, WriteAt)
{
	HANDLE hFile = CreateFileW(szTestcase, GENERIC_WRITE| GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dw = GetLastError();
	ASSERT_NE(INVALID_HANDLE_VALUE, hFile);
	IFsStream * fsStream;
	fsStream = new CFileFsStream();
	fsStream->SetFileHandle((void*)hFile);
	LARGE_INTEGER offset = {};
	ULARGE_INTEGER pos;
	ULONG size;
	unsigned char backup[10];
	unsigned char buf[10];

	offset.QuadPart = 0x200;
	ASSERT_HRESULT_SUCCEEDED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, backup, sizeof(backup), &size));

	unsigned char testcase1[10] = {
		0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
	};
	offset.QuadPart = 0x200;
	ASSERT_HRESULT_SUCCEEDED(fsStream->WriteAt(offset, IFsStream::FsStreamBegin, testcase1, 10, &size));
	ASSERT_EQ(10, size);
	ASSERT_HRESULT_SUCCEEDED(fsStream->Tell(&pos));
	ASSERT_EQ(0x200 + 10, pos.QuadPart);
	offset.QuadPart = 0x200;
	ASSERT_HRESULT_SUCCEEDED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, buf, sizeof(buf), &size));
	ASSERT_TRUE(0 == memcmp(buf, testcase1, sizeof(testcase1)));
	
	offset.QuadPart = 0x200;
	ASSERT_HRESULT_SUCCEEDED(fsStream->WriteAt(offset, IFsStream::FsStreamBegin, backup, sizeof(backup), &size));
	ASSERT_EQ(10, size);
	
	CloseHandle(hFile);
	fsStream->Release();
}