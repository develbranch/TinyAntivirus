#include <gtest/gtest.h>
#include <string.h>
#include <TinyAvCore.h>
#include "../TinyAvCore/FileSystem/FileFsEnumContext.h"
#include "../TinyAvCore/FileSystem/FileFsEnum.h"
#include "../TinyAvCore/FileSystem/FileFs.h"
#include "../TinyAvCore/FileSystem/zip/ZipFsEnum.h"

extern WCHAR szSampleDir[MAX_PATH];

class CTestEnumObserver
	: public CRefCount
	, public IFsEnumObserver
{
private:
	UINT m_Count;
public:
	CTestEnumObserver() : m_Count(0) {}
	virtual ~CTestEnumObserver() {}
	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __in void **ppvObject)
	{
		if (ppvObject == NULL) return E_INVALIDARG;
		if (IsEqualIID(riid, IID_IUnknown) ||
			IsEqualIID(riid, __uuidof(IFsEnumObserver))
			)
		{
			*ppvObject = static_cast<IFsEnumObserver*>(this);
			AddRef();
			return S_OK;
		}
		else
		{
			*ppvObject = NULL;
		}
		return E_NOINTERFACE;
	}
	DECLARE_REF_COUNT();

	UINT GetFileCount(void)
	{
		return m_Count;
	}

	virtual HRESULT WINAPI OnFileFound(__in IVirtualFs *file, __in IFsEnumContext *context, __in const int currentDepth) override
	{
		BSTR lpFileName = NULL;

		m_Count++;

		if (SUCCEEDED(file->GetFullPath(&lpFileName)))
		{
			wprintf(L"(%d) %s", currentDepth, lpFileName);
			SysFreeString(lpFileName);
		}

		printf("\n");

		file->GetFileName(&lpFileName);
		SysFreeString(lpFileName);
		file->GetFileExt(&lpFileName);
		SysFreeString(lpFileName);
		return S_OK;
	}

	virtual void WINAPI OnError(__in DWORD dwErrorCode, __in_opt LPCWSTR lpMessage = NULL) override
	{
		UNREFERENCED_PARAMETER(dwErrorCode);
		UNREFERENCED_PARAMETER(lpMessage);
	}
};

TEST(FileFsEnum, All)
{
	IFsEnum * enumObj = static_cast<IFsEnum*>(new CFileFsEnum);
	IFsEnumContext * enumContext = static_cast<IFsEnumContext*>(new CFileFsEnumContext);
	CTestEnumObserver * testObj = new CTestEnumObserver();
	IFsEnum * zip = new CZipFsEnum;
	IVirtualFs * container = static_cast<IVirtualFs*>(new CFileFs);

	if (!enumObj || !enumContext || !testObj || !zip || !container)
		goto EXIT;

	ASSERT_HRESULT_SUCCEEDED(enumContext->SetMaxDepth(-1));
	ASSERT_HRESULT_SUCCEEDED(enumContext->SetSearchPattern(L"*.*"));
	ASSERT_HRESULT_SUCCEEDED(container->Create(szSampleDir, 0));
	ASSERT_HRESULT_SUCCEEDED(enumContext->SetSearchContainer(container));
	ASSERT_HRESULT_SUCCEEDED(enumContext->SetFlags(IFsEnumContext::DetectOnly));
	ASSERT_HRESULT_SUCCEEDED(enumObj->AddObserver(testObj));
	ASSERT_HRESULT_SUCCEEDED(enumObj->AddArchiver(zip));
	ASSERT_HRESULT_SUCCEEDED(enumObj->Enum(enumContext));
	ASSERT_EQ(6, testObj->GetFileCount());
	printf("\n\n Count = %d\n", testObj->GetFileCount());
	ASSERT_HRESULT_SUCCEEDED(enumObj->RemoveArchiver(zip));
	ASSERT_HRESULT_SUCCEEDED(enumObj->RemoveObserver(testObj));

	EXIT:
		if (testObj) testObj->Release();
		if (container) container->Release();
		if (zip) zip->Release();
		if (enumContext) enumContext->Release();
		if (enumObj) enumObj->Release();
}