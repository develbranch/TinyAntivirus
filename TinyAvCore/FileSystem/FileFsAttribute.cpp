#include "FileFsAttribute.h"

CFileFsAttribute::CFileFsAttribute()
{
	m_handle = INVALID_HANDLE_VALUE;
	ZeroMemory(&m_wfd, sizeof(m_wfd));
	m_bInited = FALSE;
}

CFileFsAttribute::~CFileFsAttribute()
{
}

HRESULT WINAPI CFileFsAttribute::QueryInterface(
	__in REFIID riid,
	__out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL)
	{
		return E_INVALIDARG;
	}
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IFsAttribute)))
	{
		*ppvObject = static_cast<IFsAttribute*>(this);
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT WINAPI CFileFsAttribute::Size(__out ULARGE_INTEGER * fileSize)
{
	if (m_bInited == FALSE)
	{
		return E_NOT_SET;
	}
	if (fileSize == NULL)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = QueryAttributes();
	if (FAILED(hr)) return hr;

	fileSize->u.HighPart = m_wfd.nFileSizeHigh;
	fileSize->u.LowPart = m_wfd.nFileSizeLow;
	return hr;
}

HRESULT WINAPI CFileFsAttribute::Attributes(__out DWORD *attribs)
{
	if (m_bInited == FALSE)
	{
		return E_NOT_SET;
	}
	if (attribs == NULL)
	{
		return E_INVALIDARG;
	}
	HRESULT hr = QueryAttributes();
	if (FAILED(hr)) return hr;

	*attribs = m_wfd.dwFileAttributes;
	return hr;
}

HRESULT WINAPI CFileFsAttribute::SetAttributes(__in DWORD attribs)
{
	if (m_bInited == FALSE)
	{
		return E_NOT_SET;
	}
	if (SetFileAttributesW(m_fileName.c_str(), attribs))
	{
		return S_OK;
	}
	else
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
}

HRESULT WINAPI CFileFsAttribute::Time(
	__out_opt FILETIME *lpCreationTime,
	__out_opt FILETIME *lpLastAccessTime,
	__out_opt FILETIME *lpLastWriteTime)
{
	if (m_bInited == FALSE)
	{
		return E_NOT_SET;
	}
	if (lpCreationTime == NULL && lpLastAccessTime == NULL && lpLastWriteTime == NULL)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = QueryAttributes();
	if (FAILED(hr)) return hr;

	if (lpCreationTime) *lpCreationTime = m_wfd.ftCreationTime;
	if (lpLastAccessTime) *lpLastAccessTime = m_wfd.ftLastAccessTime;
	if (lpLastWriteTime) *lpLastWriteTime = m_wfd.ftLastWriteTime;
	return hr;
}

HRESULT WINAPI CFileFsAttribute::SetTime(
	__in_opt FILETIME *lpCreationTime,
	__in_opt FILETIME *lpLastAccessTime,
	__in_opt FILETIME *lpLastWriteTime)
{
	HRESULT hr;
	if (lpCreationTime == NULL && lpLastAccessTime == NULL && lpLastWriteTime == NULL)
	{
		return E_INVALIDARG;
	}

	if (m_handle == NULL || m_handle == INVALID_HANDLE_VALUE)
	{
		HANDLE hFile = CreateFileW(m_fileName.c_str(), FILE_WRITE_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			if (SetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime))
				hr = S_OK;
			else
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			CloseHandle(hFile);
			return hr;
		}
		return HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		if (SetFileTime(m_handle, lpCreationTime, lpLastAccessTime, lpLastWriteTime))
			return S_OK;
		else
			return HRESULT_FROM_WIN32(GetLastError());
	}
}

HRESULT WINAPI CFileFsAttribute::SetFilePath(__in LPCWSTR lpFilePath, __in_opt void* handle /*= NULL*/)
{
	m_handle = handle;
	m_fileName = lpFilePath;
	return QueryAttributes();
}

HRESULT WINAPI CFileFsAttribute::QueryAttributes(void)
{
	HANDLE hFind = FindFirstFileW(m_fileName.c_str(), &m_wfd);
	m_bInited = (hFind != INVALID_HANDLE_VALUE);
	HRESULT hr = m_bInited ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	FindClose(hFind);
	return hr;
}
