#pragma once
#include "../TinyAvBase.h"
#include "../FileSystem/FsObject.h"
#include "../FileSystem/FsEnumContext.h"

enum ScanResult
{
	NoVirus = 1,
	NotaVirus,
	VirusDetected,
	
};

enum CleanResult
{
	DonotClean = 1,
	CleanVirusSucceeded,
	CleanVirusDenied,
	VirusDeleted,
};

enum ScanAction
{
	KillVirus = 1,
	DeleteVirus,
	LeaveVirus,
};

typedef struct SCAN_RESULT
{
	ULONG scanResult;
	ULONG action;
	WCHAR malwareName[MAX_NAME];
	ULONG cleanResult;
}SCAN_RESULT, *LPSCAN_RESULT;

MIDL_INTERFACE("494B0A6D-5289-403B-B0C3-5445124E14E4")
IScanObserver : public IUnknown
{
public:
	BEGIN_INTERFACE
	
	// called when scanner started
	virtual HRESULT WINAPI OnScanStarted(__in IFsEnumContext * context) = 0;

	// called when scanner paused
	virtual HRESULT WINAPI OnScanPaused(__in IFsEnumContext * context) = 0;

	// called when scanner resumed
	virtual HRESULT WINAPI OnScanResumed(__in IFsEnumContext * context) = 0;

	// called when scanner is going to stop
	virtual HRESULT WINAPI OnScanStopping(__in IFsEnumContext * context) = 0;

	// pre-scan a file
	virtual HRESULT WINAPI OnPreScan(__in IVirtualFs * file, __in IFsEnumContext * context) = 0;

	// called when all scan-module finished
	virtual HRESULT WINAPI OnAllScanFinished(__in IVirtualFs * file, __in IFsEnumContext * context) = 0;

	//called before cleaning file
	virtual HRESULT WINAPI OnPreClean(__in IVirtualFs * file, __in IFsEnumContext * context, __inout SCAN_RESULT * result) = 0;

	//called after cleaning file
	virtual HRESULT WINAPI OnPostClean(__in IVirtualFs * file, __in IFsEnumContext * context, __in SCAN_RESULT * result) = 0;

	//called when an error occurred
	// @dwErrorCode: error code
	// @lpMessage: Error message
	virtual void WINAPI OnError(__in DWORD dwErrorCode, __in_opt LPCWSTR lpMessage = NULL) = 0;

	END_INTERFACE
};