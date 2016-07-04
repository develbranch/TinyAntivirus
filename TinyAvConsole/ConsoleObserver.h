#pragma once
#include <TinyAvCore.h>

class CConsoleObserver
	: public CRefCount
	, public IScanObserver
{
protected:
	BOOL m_bVirusDetected;
	BOOL m_bRescan;
	BOOL m_error;
	ULONGLONG m_TotalFileCnt;
	ULONGLONG m_TotalObjectCnt;
	ULONGLONG m_DetectedCnt;
	ULONGLONG m_RemovedCnt;
	ULONGLONG m_FailedCnt;

	virtual ~CConsoleObserver();

public:
	CConsoleObserver();

	// Implementing IUnknown interface
	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __in void **ppvObject);

	// Implementing IScanObserver interface
	virtual HRESULT WINAPI OnScanStarted(__in IFsEnumContext * context) override;

	virtual HRESULT WINAPI OnScanPaused(__in IFsEnumContext * context) override;

	virtual HRESULT WINAPI OnScanResumed(__in IFsEnumContext * context) override;

	virtual HRESULT WINAPI OnScanStopping(__in IFsEnumContext * context) override;

	virtual HRESULT WINAPI OnPreScan(__in IVirtualFs * file, __in IFsEnumContext * context) override;

	virtual HRESULT WINAPI OnAllScanFinished(__in IVirtualFs * file, __in IFsEnumContext * context) override;

	virtual HRESULT WINAPI OnPreClean(__in IVirtualFs * file, __in IFsEnumContext * context, __inout SCAN_RESULT * result) override;

	virtual HRESULT WINAPI OnPostClean(__in IVirtualFs * file, __in IFsEnumContext * context, __in SCAN_RESULT * result) override;

	virtual void WINAPI OnError(__in DWORD dwErrorCode, __in_opt LPCWSTR lpMessage = NULL) override;

};
