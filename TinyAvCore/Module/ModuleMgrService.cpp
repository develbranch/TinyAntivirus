#include "ModuleMgrService.h"
#include <TinyAvCore.h>

CModuleMgrService::CModuleMgrService()
{
}

CModuleMgrService::~CModuleMgrService()
{
}

HRESULT WINAPI CModuleMgrService::QueryInterface(__in REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(IModuleManager)))
	{
		*ppvObject = static_cast<IModuleManager*>(this);
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT WINAPI CModuleMgrService::Load(__in LPCWSTR lpModuleDirectory /*= NULL*/, __in LPCWSTR lpModuleName /*= NULL*/, __in DWORD flags /*= 0*/)
{
	StringW searchStr, searchPath;

	if (lpModuleDirectory)
		searchPath = lpModuleDirectory;
	else
	{
		WCHAR currentFileName[MAX_PATH + 1] = {};
		GetModuleFileNameW(NULL, currentFileName, MAX_PATH);
		searchPath = currentFileName;
	}

	if (lpModuleName == NULL)
	{
		searchStr = searchPath + L"\\*.";
		searchStr += MODULE_EXTENSION;
	}
	else
	{
		searchStr = searchPath + L"\\";
		searchStr += lpModuleName;
	}

	WIN32_FIND_DATAW wfd = {};
	HANDLE hFind = FindFirstFileW(searchStr.c_str(), &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	do
	{
		StringW modulePath = searchPath + L"\\" + wfd.cFileName;
		HMODULE handle = LoadLibraryExW(modulePath.c_str(), NULL, flags);
		if (handle)
		{
			CREATEMODULEOBJECT createModuleObj = (CREATEMODULEOBJECT)GetProcAddress(handle, MODULE_EP);
			IModule * module = NULL;
			if (SUCCEEDED(createModuleObj(CLSID_NULL, 0, __uuidof(IModule), (LPVOID*)&module)))
			{
				if (m_modules.end() == std::find(m_modules.begin(), m_modules.end(), module))
				{
					m_modules.push_back(module);
					handle = NULL;
					module = NULL;
				}
			}

			if (module) module->Release();
		}

		if (handle) FreeLibrary(handle);

	} while (FindNextFileW(hFind, &wfd));

	FindClose(hFind);

	return (m_modules.size()) ? S_OK : E_FAIL;
}

HRESULT WINAPI CModuleMgrService::Load(__in LPCWSTR lpModuleDirectory /*= NULL*/, __in ModuleType moduleType /*= 0*/, __in DWORD flags /*= 0*/)
{
	StringW searchStr, searchPath;

	if (lpModuleDirectory)
		searchPath = lpModuleDirectory;
	else
	{
		WCHAR currentFileName[MAX_PATH + 1] = {};
		GetModuleFileNameW(NULL, currentFileName, MAX_PATH);
		searchPath = currentFileName;
	}

	searchStr = searchPath + L"\\*.";
	searchStr += MODULE_EXTENSION;

	WIN32_FIND_DATAW wfd = {};
	HANDLE hFind = FindFirstFileW(searchStr.c_str(), &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	do
	{
		StringW modulePath = searchPath + L"\\" + wfd.cFileName;
		HMODULE handle = LoadLibraryExW(modulePath.c_str(), NULL, flags);
		if (handle)
		{
			CREATEMODULEOBJECT createclsObj = (CREATEMODULEOBJECT)GetProcAddress(handle, MODULE_EP);
			IModule * module = NULL;
			if (SUCCEEDED(createclsObj(CLSID_NULL, 0, __uuidof(IModule), (LPVOID*)&module)))
			{
				if (module->GetType() == moduleType)
				{
					if (m_modules.end() == std::find(m_modules.begin(), m_modules.end(), module))
					{
						m_modules.push_back(module);
						handle = NULL;
						module = NULL;
					}
				}
			}

			if (module) module->Release();
		}

		if (handle) FreeLibrary(handle);

	} while (FindNextFileW(hFind, &wfd));

	FindClose(hFind);
	return (m_modules.size()) ? S_OK : E_FAIL;
}

HRESULT WINAPI CModuleMgrService::Unload(__in LPCWSTR lpModuleName /*= NULL*/)
{
	if (lpModuleName == NULL)
	{
		for (MODULE_ARRAY::iterator it = m_modules.begin(); it != m_modules.end(); )
		{
			MODULE_INFO info;
			if (SUCCEEDED((*it)->GetModuleInfo(&info)))
			{
				(*it)->Release();
				FreeLibrary(info.handle);
				it = m_modules.erase(it);
			}
		}
		return S_OK;
	}
	else
	{
		BSTR name;
		for (MODULE_ARRAY::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
		{
			name = NULL;
			if (SUCCEEDED((*it)->GetName(&name)))
			{
				if (0 == _wcsnicmp((wchar_t const*)name, lpModuleName, MAX_NAME))
				{
					MODULE_INFO info;
					if (SUCCEEDED((*it)->GetModuleInfo(&info)))
					{
						(*it)->Release();
						FreeLibrary(info.handle);
						m_modules.erase(it);
						SysFreeString(name);
						return S_OK;
					}
				}
				SysFreeString(name);
			}
		}
	}

	return E_NOT_SET;
}

HRESULT WINAPI CModuleMgrService::Unload(__in ModuleType moduleType /*= DefaultModuleType*/)
{
	BOOL bFound = FALSE;
	for (MODULE_ARRAY::iterator it = m_modules.begin(); it != m_modules.end();)
	{
		if (moduleType == DefaultModuleType || (*it)->GetType() == moduleType)
		{
			MODULE_INFO info;
			if (SUCCEEDED((*it)->GetModuleInfo(&info)))
			{
				(*it)->Release();
				FreeLibrary(info.handle);
				it = m_modules.erase(it);
				bFound = TRUE;
			}
			else
			{
				it++;
			}
		}
	}

	return bFound ? S_OK : E_NOT_SET;
}

HRESULT WINAPI CModuleMgrService::QueryModule(__out IModule **&module, __out size_t& moduleCount, __in LPCWSTR lpModuleName /*= NULL*/)
{
	MODULE_ARRAY a;

	if (lpModuleName == NULL)
	{
		a = m_modules;
	}
	else
	{
		for (MODULE_ARRAY::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
		{
			BSTR name = NULL;
			if (SUCCEEDED((*it)->GetName(&name)) &&
				(0 == _wcsnicmp((wchar_t const*)name, lpModuleName, MAX_NAME)))
			{
				a.push_back((*it));
			}

			if (name)
				SysFreeString(name);
		}
	}

	if (a.size() == 0) return E_NOT_SET;
	moduleCount = a.size();

	module = (IModule **)CoTaskMemAlloc(a.size() * sizeof(IModule*));
	if (module == NULL)
	{
		return E_OUTOFMEMORY;
	}
	for (size_t i = 0; i < moduleCount; ++i)
	{
		module[i] = a[i];
		a[i]->AddRef();
	}
	return S_OK;
}

HRESULT WINAPI CModuleMgrService::QueryModule(__out IModule **&module, __out size_t& moduleCount, __in ModuleType moduleType /*= DefaultModuleType*/)
{
	MODULE_ARRAY a;

	if (moduleType == DefaultModuleType)
	{
		a = m_modules;
	}
	else
	{
		for (MODULE_ARRAY::iterator it = m_modules.begin(); it != m_modules.end(); ++it)
		{
			if ((*it)->GetType() == moduleType)
			{
				a.push_back((*it));
			}
		}
	}

	if (a.size() == 0) return E_NOT_SET;
	moduleCount = a.size();

	module = (IModule **)CoTaskMemAlloc(a.size() * sizeof(IModule*));
	if (module == NULL)
	{
		return E_OUTOFMEMORY;
	}
	for (size_t i = 0; i < moduleCount; ++i)
	{
		module[i] = a[i];
		a[i]->AddRef();
	}
	return S_OK;
}
