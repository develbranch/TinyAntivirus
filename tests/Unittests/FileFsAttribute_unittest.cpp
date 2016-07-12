#include <gtest/gtest.h>
#define  TEST_FILEFSATTRIBUTE
#if defined TEST_FILEFSATTRIBUTE
#include "../TinyAvCore/FileSystem/FileFsAttribute.h"
#include <string.h>

extern WCHAR szTestcase[MAX_PATH];

TEST(FileFsAttribute, Size)
{
	IFsAttribute * fsAttr = new CFileFsAttribute();
	ASSERT_HRESULT_SUCCEEDED(fsAttr->SetFilePath(szTestcase));
	ULARGE_INTEGER fileSize;
	ASSERT_HRESULT_SUCCEEDED(fsAttr->Size(&fileSize));
	ASSERT_EQ(1024*1024, fileSize.QuadPart);
	fsAttr->Release();
}
#endif