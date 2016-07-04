#define _CRT_SECURE_NO_WARNINGS
#include "ConsoleObserver.h"

CConsoleObserver::CConsoleObserver()
{
	m_bRescan = FALSE;
	m_bVirusDetected = FALSE;
	m_TotalFileCnt = 0;
	m_TotalObjectCnt = 0;
	m_DetectedCnt = 0;
	m_RemovedCnt = 0;
	m_error = FALSE;
}

CConsoleObserver::~CConsoleObserver()
{
}

HRESULT WINAPI CConsoleObserver::QueryInterface(__in REFIID riid, __in void **ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;

	if (riid == IID_IUnknown ||
		riid == __uuidof(IScanObserver))
	{
		*ppvObject = static_cast<IScanObserver*>(this);
		AddRef();
		return S_OK;
	}
	else
	{
		*ppvObject = NULL;
	}
	return E_NOINTERFACE;
}

HRESULT WINAPI CConsoleObserver::OnScanStarted(__in IFsEnumContext * context)
{
	printf("Scanning ...\n");
	m_TotalFileCnt = 0;
	m_TotalObjectCnt = 0;
	m_DetectedCnt = 0;
	m_RemovedCnt = 0;
	m_FailedCnt = 0;
	return S_OK;
}

HRESULT WINAPI CConsoleObserver::OnScanPaused(__in IFsEnumContext * context)
{
	printf("Paused\n");
	return S_OK;
}

HRESULT WINAPI CConsoleObserver::OnScanResumed(__in IFsEnumContext * context)
{
	printf("Resumed\n");
	return S_OK;
}

HRESULT WINAPI CConsoleObserver::OnScanStopping(__in IFsEnumContext * context)
{
	printf("\n=============================================\n");
	printf("Scanned       : %lld file(s) (%lld object(s))\n", m_TotalFileCnt, m_TotalObjectCnt);
	printf("Detected      : %lld file(s)\n", m_DetectedCnt);
	printf("Removed       : %lld file(s)\n", m_RemovedCnt);
	printf("Access denied : %lld file(s)\n", m_FailedCnt);
	return S_OK;
}

HRESULT WINAPI CConsoleObserver::OnPreScan(__in IVirtualFs * file, __in IFsEnumContext * context)
{
	m_error = FALSE;
	m_TotalObjectCnt++;
	if (m_bRescan) return S_OK;
	BSTR fullPath = NULL;
	ULONG fsType;
	if (SUCCEEDED(file->GetFsType(&fsType)) &&
		fsType == IVirtualFs::basic)
	{
		m_TotalFileCnt++;
	}

	file->GetFullPath(&fullPath);
	if (fullPath)
	{
		WCHAR wzDisplay[70] = {};
		if (wcslen(fullPath) < _countof(wzDisplay))
		{
			wcscpy_s(wzDisplay, _countof(wzDisplay), fullPath);
		}
		else
		{
			wcsncpy(wzDisplay, fullPath, 20);
			wcscat(wzDisplay, L"...");

			wcscpy_s(&wzDisplay[wcslen(wzDisplay)], _countof(wzDisplay) - wcslen(wzDisplay), &fullPath[wcslen(fullPath) - (_countof(wzDisplay) - 1 - wcslen(wzDisplay))]);
		}
		wprintf(L"%-70s  ", wzDisplay);
		SysFreeString(fullPath);
	}
	return S_OK;
}

HRESULT WINAPI CConsoleObserver::OnAllScanFinished(__in IVirtualFs * file, __in IFsEnumContext * context)
{
	if (m_bVirusDetected == FALSE && !m_error)
	{
		printf("OK\n");
	}
	m_bVirusDetected = FALSE;
	m_bRescan = FALSE;
	m_error = FALSE;
	return S_OK;
}

HRESULT WINAPI CConsoleObserver::OnPreClean(__in IVirtualFs * file, __in IFsEnumContext * context, __inout SCAN_RESULT * result)
{
	wprintf(L"\n\t%s ", result->malwareName);
	result->action = KillVirus;
	m_bVirusDetected = TRUE;
	m_DetectedCnt++;
	return S_OK;
}

HRESULT WINAPI CConsoleObserver::OnPostClean(__in IVirtualFs * file, __in IFsEnumContext * context, __in SCAN_RESULT * result)
{
	if (result && result->scanResult == VirusDetected)
	{
		switch (result->cleanResult)
		{
		case CleanVirusSucceeded:
			printf("Disinfected \n");
			m_bRescan = TRUE;
			m_RemovedCnt++;
			break;

		case CleanVirusDenied:
			printf("Access Denied \n");
			m_FailedCnt++;
			break;

		case VirusDeleted:
			printf("Deleted \n");
			m_RemovedCnt++;
			break;

		default:
			printf("Unknown error \n");
			m_FailedCnt++;
			break;
		}
	}

	return S_OK;
}

void WINAPI CConsoleObserver::OnError(__in DWORD dwErrorCode, __in_opt LPCWSTR lpMessage /*= NULL*/)
{
	m_error = TRUE;
	printf("\n[!] ");
	if (lpMessage)
	{
		wprintf(L"%s ", lpMessage);
	}

	switch (dwErrorCode)
	{
	case IFsEnum::FsEnumAccessDenied:
		wprintf(L"Access denied.");
		m_TotalFileCnt++;
		m_FailedCnt++;
		break;
	case IFsEnum::FsEnumNotFound:
		wprintf(L"Not found.");
		break;

	case IEmulObserver::EmulatorIsNotFound:
		wprintf(L"unicorn.dll and its dependent dlls are needed.");
		break;
	case IEmulObserver::EmulatorIsNotRunable:
		wprintf(L"Internal error has occurred. Skip this file.");
		break;
	case IEmulObserver::EmulatorInternalError:
		wprintf(L"Emulator has internal errors. Skip this file.");
		break;
	default:
		break;
	}

	printf("\n");
}
