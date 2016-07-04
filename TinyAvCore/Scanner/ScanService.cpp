#include "ScanService.h"
#include "..\FileSystem\FileFsEnum.h"
#include "..\FileSystem\FileFsEnumContext.h"
#include "..\FileSystem\FileFs.h"
#include "..\FileSystem\zip\ZipFsEnum.h"

SCAN_CONTEXT_MAP CScanService::m_ContextMap;

CScanService::CScanService()
{
}

CScanService::~CScanService()
{
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		m_Observers[i]->Release();
	}

	n = m_ScanModules.size();
	for (i = 0; i < n; i++)
	{
		m_ScanModules[i]->OnScanShutdown();
		m_ScanModules[i]->Release();
	}
}

HRESULT WINAPI CScanService::QueryInterface(
	__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(IScanner)))
	{
		*ppvObject = static_cast<IScanner*>(this);
		this->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT WINAPI CScanService::AddScanObserver(__in IScanObserver *observer)
{
	if (observer == NULL) return E_INVALIDARG;
	if (m_Observers.end() == std::find(m_Observers.begin(), m_Observers.end(), observer))
	{
		observer->AddRef();
		m_Observers.push_back(observer);
		return S_OK;
	}

	return E_NOT_VALID_STATE;
}

HRESULT WINAPI CScanService::RemoveScanObserver(__in IScanObserver *observer)
{
	if (observer == NULL) return E_INVALIDARG;
	std::vector<IScanObserver *>::iterator it;
	it = std::find(m_Observers.begin(), m_Observers.end(), observer);
	if (m_Observers.end() == it)
	{
		return E_NOT_SET;
	}

	(*it)->Release();
	m_Observers.erase(it);
	return S_OK;
}

HRESULT WINAPI CScanService::AddScanModule(__in IScanModule *scanModule)
{
	if (scanModule == NULL) return E_INVALIDARG;
	if (m_ScanModules.end() == std::find(m_ScanModules.begin(), m_ScanModules.end(), scanModule))
	{
		HRESULT hr = scanModule->OnScanInitialize();
		if (SUCCEEDED(hr))
		{
			scanModule->AddRef();
			m_ScanModules.push_back(scanModule);
			return S_OK;
		}
		else
		{
			scanModule->OnScanShutdown();
		}
		return hr;
	}

	return E_NOT_VALID_STATE;
}

HRESULT WINAPI CScanService::RemoveScanModule(__in IScanModule *scanModule)
{
	if (scanModule == NULL) return E_INVALIDARG;
	std::vector<IScanModule *>::iterator it;
	it = std::find(m_ScanModules.begin(), m_ScanModules.end(), scanModule);
	if (m_ScanModules.end() == it)
	{
		return E_NOT_SET;
	}

	(*it)->OnScanShutdown();
	(*it)->Release();
	m_ScanModules.erase(it);
	return S_OK;
}

HRESULT WINAPI CScanService::Start(__in IFsEnumContext *enumContext)
{
	HRESULT hr;
	if (m_ContextMap.size() >= MAXIMUM_WAIT_OBJECTS)
		return E_NOT_VALID_STATE;
	SCAN_THREAD_PARAM * scanParam = new SCAN_THREAD_PARAM;
	if (scanParam == NULL) return E_OUTOFMEMORY;

	scanParam->stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (scanParam->stopEvent == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		delete scanParam;
		return hr;
	}
	scanParam->threadHandle = CreateThread(NULL, 0, &CScanService::ScanThread, scanParam, CREATE_SUSPENDED, NULL);
	if (scanParam->threadHandle == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		CloseHandle(scanParam->stopEvent);
		delete scanParam;
		return hr;
	}
	scanParam->enumurate = NULL;
	scanParam->enumContext = enumContext;
	enumContext->AddRef();
	scanParam->instance = this;
	m_ContextMap[enumContext] = scanParam;
	ResumeThread(scanParam->threadHandle);
	return S_OK;
}

HRESULT WINAPI CScanService::Stop(__in IFsEnumContext *enumContext)
{
	if (m_ContextMap.find(enumContext) == m_ContextMap.end())  return E_NOT_SET;
	if (m_ContextMap[enumContext] == NULL) return E_NOT_SET;
	if (m_ContextMap[enumContext]->threadHandle == NULL) return E_NOT_VALID_STATE;

	if (!SetEvent(m_ContextMap[enumContext]->stopEvent))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	if (m_ContextMap[enumContext]->enumurate)
	{
		m_ContextMap[enumContext]->enumurate->Stop();
		return S_OK;
	}
	else
	{
		return E_NOT_SET;
	}
}

HRESULT WINAPI CScanService::Pause(__in IFsEnumContext *enumContext)
{
	HRESULT hr;
	size_t i, n;

	if (m_ContextMap.find(enumContext) == m_ContextMap.end())  return E_NOT_SET;
	if (m_ContextMap[enumContext] == NULL) return E_NOT_SET;
	if (m_ContextMap[enumContext]->threadHandle == NULL) return E_NOT_VALID_STATE;
	SuspendThread(m_ContextMap[enumContext]->threadHandle);

	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnScanPaused(enumContext);
		if (FAILED(hr)) return hr;
	}
	return S_OK;
}

HRESULT WINAPI CScanService::Resume(__in IFsEnumContext *enumContext)
{
	HRESULT hr;
	size_t i, n;

	if (m_ContextMap.find(enumContext) == m_ContextMap.end()) return E_NOT_SET;
	if (m_ContextMap[enumContext] == NULL) return E_NOT_SET;
	if (m_ContextMap[enumContext]->threadHandle == NULL) return E_NOT_VALID_STATE;
	ResumeThread(m_ContextMap[enumContext]->threadHandle);

	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnScanResumed(enumContext);
		if (FAILED(hr)) return hr;
	}
	return S_OK;
}

DWORD WINAPI CScanService::ScanThread(__in LPVOID lpParam)
{
	if (lpParam == NULL)  return 0;
	SCAN_THREAD_PARAM * param = (SCAN_THREAD_PARAM*)lpParam;
	param->instance->OnScanThread(param);
	return 0;
}

void WINAPI CScanService::OnScanThread(__in SCAN_THREAD_PARAM * param)
{
	if (!param) return;

	HRESULT hr;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnScanStarted(param->enumContext);
		if (FAILED(hr))
		{
			return;
		}
	}

	param->enumurate = static_cast<IFsEnum*>(new CFileFsEnum);
	if (param->enumurate == NULL)
		return;

	param->enumurate->AddObserver(static_cast<IFsEnumObserver*>(param->instance));
	AddArchivers(param->enumurate);
	param->enumurate->Enum(param->enumContext);
	param->enumurate->Release();
	param->enumurate = NULL;

	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnScanStopping(param->enumContext);
		if (FAILED(hr)) return;
	}

	m_ContextMap.erase(param->enumContext);
	param->enumContext->Release();
	delete param;
}

void WINAPI CScanService::AddArchivers(__inout IFsEnum * enumurate)
{
	if (enumurate == NULL) return;
	IFsEnum * archiver = static_cast<IFsEnum *>(new CZipFsEnum);
	if (archiver == NULL) return;
	enumurate->AddArchiver(archiver);
	archiver->Release();
}

HRESULT WINAPI CScanService::OnFileFound(__in IVirtualFs *file, __in IFsEnumContext *context, __in const int currentDepth)
{
	UNREFERENCED_PARAMETER(currentDepth);
	HRESULT hr = S_OK;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; )
	{
		hr = m_ScanModules[i]->Scan(file, context, this);
		if (m_ContextMap.find(context) != m_ContextMap.end())
		{
			if (WaitForSingleObject(m_ContextMap[context]->stopEvent, 0) == WAIT_OBJECT_0)
			{
				return hr;
			}
		}

		if (hr == E_NOT_SET) break; // file is deleted.
		if (hr == S_FALSE)			// file is disinfected. Rescan file.
		{
			i = 0;
			continue;
		}
		if (FAILED(hr))
		{
			OnAllScanFinished(file, context);
			return hr;
		}
		ULONG flags;
		if (SUCCEEDED(file->GetFlags(&flags)) && 
			TEST_FLAG(flags, IVirtualFs::fsDeferredDeletion))
		{
			break;
		}

		i++;
	}

	OnAllScanFinished(file, context);
	return hr;
}

HRESULT WINAPI CScanService::OnScanStarted(__in IFsEnumContext * context)
{
	HRESULT hr;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnScanStarted(context);
		if (FAILED(hr)) return hr;
	}

	return S_OK;
}

HRESULT WINAPI CScanService::OnScanPaused(__in IFsEnumContext * context)
{
	HRESULT hr;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnScanPaused(context);
		if (FAILED(hr)) return hr;
	}

	return S_OK;
}

HRESULT WINAPI CScanService::OnScanResumed(__in IFsEnumContext * context)
{
	HRESULT hr;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnScanResumed(context);
		if (FAILED(hr)) return hr;
	}

	return S_OK;
}

HRESULT WINAPI CScanService::OnScanStopping(__in IFsEnumContext * context)
{
	HRESULT hr;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnScanStopping(context);
		if (FAILED(hr)) return hr;
	}

	return S_OK;
}

HRESULT WINAPI CScanService::OnPreScan(__in IVirtualFs * file, __in IFsEnumContext * context)
{
	HRESULT hr;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnPreScan(file, context);
		if (FAILED(hr)) return hr;
	}

	return S_OK;
}

HRESULT WINAPI CScanService::OnAllScanFinished(__in IVirtualFs * file, __in IFsEnumContext * context)
{
	HRESULT hr;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnAllScanFinished(file, context);
		if (FAILED(hr)) return hr;
	}

	return S_OK;
}

HRESULT WINAPI CScanService::OnPreClean(__in IVirtualFs * file, __in IFsEnumContext * context, __inout SCAN_RESULT * result)
{
	HRESULT hr;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnPreClean(file, context, result);
		if (FAILED(hr)) return hr;
	}

	return S_OK;
}

HRESULT WINAPI CScanService::OnPostClean(__in IVirtualFs * file, __in IFsEnumContext * context, __in SCAN_RESULT * result)
{
	HRESULT hr;
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		hr = m_Observers[i]->OnPostClean(file, context, result);
		if (FAILED(hr)) return hr;
	}

	return S_OK;
}

void WINAPI CScanService::OnError(__in DWORD dwErrorCode, __in_opt LPCWSTR lpMessage /*= NULL*/)
{
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		m_Observers[i]->OnError(dwErrorCode, lpMessage);
	}
}

void WINAPI CScanService::Forever(void)
{
	HANDLE waitTable[MAXIMUM_WAIT_OBJECTS] = {};
	size_t n = m_ContextMap.size();
	if (n > MAXIMUM_WAIT_OBJECTS)
		n = MAXIMUM_WAIT_OBJECTS;
	size_t i = 0;
	for (SCAN_CONTEXT_MAP::iterator it = m_ContextMap.begin(); it != m_ContextMap.end(); ++it)
	{
		SCAN_THREAD_PARAM *param = it->second;
		if (i < n)
		{
			waitTable[i++] = param->threadHandle;
		}
		else
			break;
	}

	WaitForMultipleObjects((DWORD)n, waitTable, TRUE, INFINITE);
}
