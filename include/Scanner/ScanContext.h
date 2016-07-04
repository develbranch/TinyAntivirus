#pragma once
#include "../TinyAvBase.h"

MIDL_INTERFACE("C832DE40-EEF1-4A5C-982C-7E3FA1E6F8F1")
IScanContext: public IUnknown
{
public:
	BEGIN_INTERFACE
	virtual HRESULT WINAPI AddScanPath(__in LPCWSTR lpPath) = 0;
	virtual HRESULT WINAPI RemoveScanPath(__in LPCWSTR lpPath) = 0;
	virtual HRESULT WINAPI GetScanList(__out BSTR* lpPath, __out UINT *itemCount) = 0;
	virtual HRESULT WINAPI FreeScanList(__in BSTR lpPath, __in UINT itemCount) = 0;

	virtual HRESULT WINAPI AddIgnoreItem(__in LPCWSTR lpPath) = 0;
	virtual HRESULT WINAPI RemoveIgnoreItem(__in LPCWSTR lpPath) = 0;
	virtual HRESULT WINAPI GetIgnoreList(__out BSTR* lpPath, __out UINT *itemCount) = 0;
	virtual HRESULT WINAPI FreeIgnoreList(__in BSTR lpPath, __in UINT itemCount) = 0;

	virtual HRESULT WINAPI AddScanPattern(__in LPCWSTR lpPattern) = 0;
	virtual HRESULT WINAPI RemoveScanPattern(__in LPCWSTR lpPattern) = 0;
	virtual HRESULT WINAPI GetScanPattern(__out BSTR* lpPath, __out UINT *itemCount) = 0;
	virtual HRESULT WINAPI FreeScanPattern(__in BSTR lpPath, __in UINT itemCount) = 0;

	virtual HRESULT WINAPI SetMaxDepth(__in int maxDepth) = 0;
	virtual int WINAPI GetMaxDepth(void) = 0;
	
	virtual HRESULT WINAPI SetMaxFileSize(__in ULARGE_INTEGER fileSize) = 0;
	virtual HRESULT WINAPI GetMaxFileSize(__in ULARGE_INTEGER *fileSize) = 0;

	virtual UINT WINAPI GetScanMode(void) = 0;
	virtual HRESULT WINAPI SetScanMode(__in UINT status) = 0;
	END_INTERFACE
};