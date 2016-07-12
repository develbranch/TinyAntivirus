#include "FileFs.h"
#include "FileFsAttribute.h"
#include "FileFsStream.h"

CFileFs::CFileFs()
{
	m_flags = 0;
	m_handle = INVALID_HANDLE_VALUE;
	m_container = NULL;
	m_error = 0;
	m_attribute = static_cast<IFsAttribute*> (new CFileFsAttribute());
	m_stream = static_cast<IFsStream*> (new CFileFsStream());
	m_delimiter = StringW(L"\\");
	m_fsType = IFsType::basic;
}

CFileFs::~CFileFs()
{
	BSTR fullPath = NULL;
	Close();

	if (TEST_FLAG(m_flags, fsDeferredDeletion))
	{
		GetFullPath(&fullPath);
	}

	if (m_attribute)
	{
		m_attribute->Release();
		m_attribute = NULL;
	}

	if (m_stream)
	{
		m_stream->Release();
		m_stream = NULL;
	}

	if (m_container)
	{
		m_container->Release();
		m_container = NULL;
	}

	if (fullPath)
	{
		if (!DeleteFileW(fullPath))
		{
			// delete when the system restarts. 
			MoveFileExW(fullPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
		SysFreeString(fullPath);
	}
}

HRESULT WINAPI CFileFs::QueryInterface(
	__in REFIID riid,
	__out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IVirtualFs)))
	{
		AddRef();
		*ppvObject = static_cast<IVirtualFs*>(this);
		return S_OK;
	}

	else if (IsEqualIID(riid, __uuidof(IFsStream)))
	{
		if (m_stream == NULL) return E_NOT_SET;
		m_stream->AddRef();
		*ppvObject = static_cast<IFsStream*>(m_stream);
		return S_OK;
	}

	else if (IsEqualIID(riid, __uuidof(IFsAttribute)))
	{
		if (m_attribute == NULL) return E_NOT_SET;
		m_attribute->AddRef();
		*ppvObject = static_cast<IFsAttribute*>(m_attribute);
		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT WINAPI CFileFs::Create(__in LPCWSTR lpFileName, __in ULONG const flags)
{
	if (!m_FileName.empty()) return E_NOT_VALID_STATE;
	if (lpFileName == NULL || _tcslen(lpFileName) == 0) return E_INVALIDARG;
	m_FileName = lpFileName;
	BSTR fullPath;
	HRESULT hr = GetFullPath(&fullPath);
	if (FAILED(hr))
		return hr;

	if (m_attribute)
	{
		m_attribute->SetFilePath(fullPath);
	}
	m_flags = 0;
	m_handle = INVALID_HANDLE_VALUE;

	DWORD dwDesiredAccess = 0, dwShareMode = 0, dwCreationDisposition = 0, dwFlagsAndAttributes = 0;

	m_flags = flags;
	hr = S_OK;
	if (m_flags > 0)
	{
		dwDesiredAccess |= TEST_FLAG(m_flags, fsRead) ? GENERIC_READ : 0;
		dwDesiredAccess |= TEST_FLAG(m_flags, fsWrite) ? GENERIC_WRITE : 0;

		dwShareMode |= TEST_FLAG(m_flags, fsSharedRead) ? FILE_SHARE_READ : 0;
		dwShareMode |= TEST_FLAG(m_flags, fsSharedWrite) ? FILE_SHARE_WRITE : 0;
		dwShareMode |= TEST_FLAG(m_flags, fsSharedDelete) ? FILE_SHARE_DELETE : 0;

		dwFlagsAndAttributes |= TEST_FLAG(m_flags, fsAttrNormal) ? FILE_ATTRIBUTE_NORMAL : 0;
		dwFlagsAndAttributes |= TEST_FLAG(m_flags, fsAttrReadonly) ? FILE_ATTRIBUTE_READONLY : 0;
		dwFlagsAndAttributes |= TEST_FLAG(m_flags, fsAttrSystem) ? FILE_ATTRIBUTE_SYSTEM : 0;
		dwFlagsAndAttributes |= TEST_FLAG(m_flags, fsAttrHidden) ? FILE_ATTRIBUTE_HIDDEN : 0;
		dwFlagsAndAttributes |= TEST_FLAG(m_flags, fsAttrTemporary) ? FILE_ATTRIBUTE_TEMPORARY : 0;
		dwFlagsAndAttributes |= TEST_FLAG(m_flags, fsAttrDeleteOnClose) ? FILE_FLAG_DELETE_ON_CLOSE : 0;

		if (TEST_FLAG(m_flags, fsCreateNew))
		{
			dwCreationDisposition = CREATE_NEW;
		}
		else if (TEST_FLAG(m_flags, fsCreateAlways))
		{
			dwCreationDisposition = CREATE_ALWAYS;
		}
		else if (TEST_FLAG(m_flags, fsOpenAlways))
		{
			dwCreationDisposition = OPEN_ALWAYS;
		}
		else if (TEST_FLAG(m_flags, fsOpenExisting))
		{
			dwCreationDisposition = OPEN_EXISTING;
		}
		else
		{
			hr = E_INVALIDARG;
		}

		if (SUCCEEDED(hr))
		{
			BOOL deferredCreation = TEST_FLAG(m_flags, fsDeferredCreation);
			if (!deferredCreation)
			{
				dwFlagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;

				m_handle = CreateFileW(fullPath, dwDesiredAccess, dwShareMode,
					NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);

				if (m_handle == INVALID_HANDLE_VALUE)
					hr = HRESULT_FROM_WIN32(GetLastError());

				if (SUCCEEDED(hr))
					m_stream->SetFileHandle((void*)m_handle);
			}
			else
			{
				CLR_FLAG(m_flags, fsDeferredCreation);
				hr = S_OK;
			}
		}
	}

	SysFreeString(fullPath);
	m_error = (ULONG)(hr & 0xffff);
	return hr;
}

HRESULT WINAPI CFileFs::Close(void)
{
	HRESULT hr = S_OK;
	if (m_handle != INVALID_HANDLE_VALUE && m_handle != NULL)
	{
		if (!CloseHandle(m_handle))
			hr = HRESULT_FROM_WIN32(GetLastError());
		else
			hr = S_OK;
	}

	m_handle = INVALID_HANDLE_VALUE;

	if (TEST_FLAG(m_flags, fsDeferredDeletion))
	{
		m_flags = fsDeferredDeletion;
	}
	else
	{
		m_flags = 0;
		m_FileName.clear();
	}

	if (m_stream)
		m_stream->SetFileHandle((void*)m_handle);
	m_error = (ULONG)(hr & 0xffff);
	return hr;
}

HRESULT WINAPI CFileFs::IsOpened(__out BOOL *isOpened)
{
	if (!isOpened) return E_INVALIDARG;
	*isOpened = (m_handle != INVALID_HANDLE_VALUE && m_handle != NULL);
	return S_OK;
}

HRESULT WINAPI CFileFs::ReCreate(__in_opt void* handle /*= NULL*/, __in_opt ULONG const flags /*= 0*/)
{
	StringW fileName = m_FileName;

	Close();
	if (handle == NULL)
		return Create(fileName.c_str(), flags ? flags : m_flags);
	else
	{
		m_handle = (HANDLE)handle;
		if (flags)
			m_flags = flags;
		return S_OK;
	}
}

HRESULT WINAPI CFileFs::GetFlags(__out ULONG *flags)
{
	if (!flags) return E_INVALIDARG;
	*flags = m_flags;
	return S_OK;
}

HRESULT WINAPI CFileFs::GetFullPath(__out BSTR *fullPath)
{
	if (fullPath == NULL) return E_INVALIDARG;
	if (m_FileName.length() == 0) return E_NOT_SET;

	BSTR containerFullPath = NULL;
	if (m_container)
	{
		m_container->GetFullPath(&containerFullPath);
	}

	StringW fullName;

	if (containerFullPath)
	{
		fullName = containerFullPath + m_delimiter + m_FileName;
		SysFreeString(containerFullPath);
	}
	else
	{
		WIN32_FIND_DATAW wfd;
		HANDLE hFind = FindFirstFileW(m_FileName.c_str(), &wfd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			fullName = m_FileName;
			FindClose(hFind);
		}
		else
		{
			DWORD dwRequiredSize = MAX_PATH;
			for (;;)
			{
				LPWSTR lpBuffer = new WCHAR[dwRequiredSize];
				if (lpBuffer == NULL)
				{
					return E_OUTOFMEMORY;
				}

				DWORD dwRet = GetCurrentDirectoryW(dwRequiredSize, lpBuffer);
				if (dwRet > dwRequiredSize)
				{
					dwRequiredSize = dwRet;
					delete[] lpBuffer;
					continue;
				}

				fullName = lpBuffer + m_delimiter + m_FileName;
				delete[] lpBuffer;
				break;
			}
		}
	}

	*fullPath = SysAllocString(fullName.c_str());
	return *fullPath ? S_OK : E_OUTOFMEMORY;
}

HRESULT WINAPI CFileFs::GetFileName(__out BSTR *fileName)
{
	if (fileName == NULL) return E_INVALIDARG;
	if (m_FileName.length() == 0) return E_NOT_SET;
	size_t pos = m_FileName.rfind(L'\\');
	if (pos == StringW::npos)
	{
		*fileName = SysAllocString(m_FileName.c_str());
		return *fileName ? S_OK : E_OUTOFMEMORY;
	}
	else
	{
		if (m_FileName.length() - pos - 1 == 0) return E_NOT_VALID_STATE;
		StringW name = m_FileName.substr(pos + 1, m_FileName.length() - pos - 1);
		*fileName = SysAllocString(name.c_str());
		return *fileName ? S_OK : E_OUTOFMEMORY;
	}
}

HRESULT WINAPI CFileFs::GetFileExt(__out BSTR *fileExt)
{
	if (fileExt == NULL) return E_INVALIDARG;
	if (m_FileName.length() == 0) return E_NOT_SET;
	size_t pos = m_FileName.rfind(L'.');
	if (pos == StringW::npos) return E_NOT_VALID_STATE;
	if (m_FileName.length() - pos - 1 == 0) return E_NOT_SET;
	StringW ext = m_FileName.substr(pos + 1, m_FileName.length() - pos - 1);

	*fileExt = SysAllocString(ext.c_str());
	return *fileExt ? S_OK : E_OUTOFMEMORY;
}

HRESULT WINAPI CFileFs::GetContainer(__out IVirtualFs **container)
{
	if (container == NULL) return E_INVALIDARG;
	if (m_container == NULL) return E_NOT_SET;
	m_container->AddRef();
	*container = m_container;
	return S_OK;
}

HRESULT WINAPI CFileFs::SetContainer(__in IVirtualFs *container)
{
	if (m_container != NULL)
	{
		m_container->Release();
		m_container = NULL;
	}
	m_container = container;
	if (container)
		container->AddRef();
	return S_OK;
}

ULONG WINAPI CFileFs::GetError(void)
{
	return m_error;
}

void WINAPI CFileFs::SetError(__in const ULONG error)
{
	m_error = error;
}

HRESULT WINAPI CFileFs::GetHandle(__out LPVOID * fileHandle)
{
	if (fileHandle == NULL) return E_INVALIDARG;
	if (m_handle == NULL || m_handle == INVALID_HANDLE_VALUE) return E_NOT_SET;
	*fileHandle = (LPVOID)m_handle;
	return S_OK;
}

HRESULT WINAPI CFileFs::GetFsType(__out ULONG * fsType)
{
	if (fsType == NULL) return E_INVALIDARG;
	*fsType = m_fsType;
	return S_OK;
}

HRESULT WINAPI CFileFs::DeferredDelete(void)
{
	m_flags |= fsDeferredDeletion;
	return S_OK;
}