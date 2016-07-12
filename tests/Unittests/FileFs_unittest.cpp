#include <string.h>
#include <gtest/gtest.h>
#include "../TinyAvCore/FileSystem/FileFs.h"

extern WCHAR szTestcase[MAX_PATH];

TEST(FileFs, All)
{
	ULONG flags;
	BOOL isOpened;
	WCHAR szNewFile[MAX_PATH];
	wcscpy_s(szNewFile, MAX_PATH, szTestcase);
	wcscat_s(szNewFile, MAX_PATH, L".new");
	CopyFileW(szTestcase, szNewFile, FALSE);
	IVirtualFs * fs = new CFileFs();
	ULONG creationFlags = IVirtualFs::fsRead | IVirtualFs::fsSharedRead | IVirtualFs::fsSharedWrite | IVirtualFs::fsOpenExisting | IVirtualFs::fsAttrNormal;
	
	ASSERT_HRESULT_SUCCEEDED(fs->Create(szNewFile, creationFlags));
	ASSERT_HRESULT_SUCCEEDED(fs->IsOpened(&isOpened));
	ASSERT_TRUE(isOpened);
	ASSERT_HRESULT_SUCCEEDED(fs->Close());

	ASSERT_HRESULT_SUCCEEDED(fs->Create(szNewFile, 0));
	ASSERT_HRESULT_SUCCEEDED(fs->IsOpened(&isOpened));
	ASSERT_FALSE(isOpened);
	ASSERT_HRESULT_SUCCEEDED(fs->ReCreate(NULL, creationFlags));
	ASSERT_HRESULT_SUCCEEDED(fs->IsOpened(&isOpened));
	ASSERT_TRUE(isOpened);
	ASSERT_HRESULT_SUCCEEDED(fs->GetFlags(&flags));
	ASSERT_EQ(creationFlags, flags);
	BSTR s;
	ASSERT_HRESULT_SUCCEEDED(fs->GetFullPath(&s));
	ASSERT_STREQ(szNewFile, s);
	SysFreeString(s);
	ASSERT_HRESULT_SUCCEEDED(fs->GetFileName(&s));
	ASSERT_STREQ(TEXT("testcase.bin.new"), s);
	SysFreeString(s);
	ASSERT_HRESULT_SUCCEEDED(fs->GetFileExt(&s));
	ASSERT_STREQ(TEXT("new"), s);
	SysFreeString(s);
	ULONG fsType;
	ASSERT_HRESULT_SUCCEEDED(fs->GetFsType(&fsType));
	ASSERT_EQ(IVirtualFs::basic, fsType);
	ASSERT_HRESULT_SUCCEEDED(fs->DeferredDelete());
	ASSERT_HRESULT_SUCCEEDED(fs->Close());
	fs->Release();
}