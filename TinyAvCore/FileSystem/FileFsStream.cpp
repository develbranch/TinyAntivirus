#include "FileFsStream.h"

CFileFsStream::CFileFsStream()
{
	m_hFile = INVALID_HANDLE_VALUE;
	ZeroMemory(&m_currentPos, sizeof(m_currentPos));
	m_cacheSize = 0;
	m_cache = new char[DEFAULT_MAX_CACHE_SIZE];
	ZeroMemory(&m_cachePos, sizeof(m_cachePos));
}

CFileFsStream::~CFileFsStream()
{
	if (m_cache)
	{
		delete[] m_cache;
		m_cache = NULL;
	}
}

HRESULT WINAPI CFileFsStream::QueryInterface(
	__in REFIID riid,
	__out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IFsStream)))
	{
		*ppvObject = static_cast<IFsStream*>(this);
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT WINAPI CFileFsStream::Read(
	__out LPVOID buffer,
	__in ULONG bufferSize,
	__out_opt ULONG * readSize)
{
	ULONG r;
	if (m_hFile == INVALID_HANDLE_VALUE) return E_NOT_SET;
	if (buffer == NULL || bufferSize == 0) return E_INVALIDARG;

	if ((m_cachePos.QuadPart <= m_currentPos.QuadPart) &&
		(m_currentPos.QuadPart + bufferSize < m_cachePos.QuadPart + m_cacheSize))
	{
		memcpy(buffer, &m_cache[m_currentPos.QuadPart - m_cachePos.QuadPart], bufferSize);
		m_currentPos.QuadPart += bufferSize;
		LARGE_INTEGER distanceToMove;
		distanceToMove.QuadPart = m_currentPos.QuadPart;
		if (readSize) *readSize = bufferSize;

		if (FALSE == SetFilePointerEx(m_hFile, distanceToMove, (PLARGE_INTEGER)NULL, FILE_BEGIN))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}
	else
	{
		if (!ReadFile(m_hFile, buffer, bufferSize, &r, NULL))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		if (r)
		{
			m_cacheSize = r < DEFAULT_MAX_CACHE_SIZE ? r : DEFAULT_MAX_CACHE_SIZE;
			memcpy(m_cache, buffer, m_cacheSize);
		}

		m_cachePos.QuadPart = m_currentPos.QuadPart;
		m_currentPos.QuadPart += r;

		if (readSize) *readSize = r;
	}

	return S_OK;
}

HRESULT WINAPI CFileFsStream::Write(
	__in const void * buffer,
	__in ULONG bufferSize,
	__out_opt ULONG * writtenSize)
{
	ULONG w;
	if (m_hFile == INVALID_HANDLE_VALUE) return E_NOT_SET;
	if (buffer == NULL || bufferSize == 0) return E_INVALIDARG;

	// write to disk
	if (!WriteFile(m_hFile, buffer, bufferSize, &w, NULL))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// update cache
	if ((m_cachePos.QuadPart < m_currentPos.QuadPart) &&
		(m_currentPos.QuadPart + w < m_cachePos.QuadPart + m_cacheSize))
	{
		memcpy(&m_cache[m_currentPos.QuadPart - m_cachePos.QuadPart], buffer, w);
		m_currentPos.QuadPart += w;
	}
	else
	{
		m_cacheSize = w < DEFAULT_MAX_CACHE_SIZE ? w : DEFAULT_MAX_CACHE_SIZE;
		memcpy(m_cache, buffer, m_cacheSize);
		m_cachePos.QuadPart = m_currentPos.QuadPart;
		m_currentPos.QuadPart += w;
	}

	if (writtenSize) *writtenSize = w;
	return S_OK;
}

HRESULT WINAPI CFileFsStream::Tell(__out ULARGE_INTEGER * pos)
{
	if (m_hFile == INVALID_HANDLE_VALUE) return E_NOT_SET;
	if (pos == NULL) return E_INVALIDARG;

	*pos = m_currentPos;
	return S_OK;
}

HRESULT WINAPI CFileFsStream::Seek(
	__out_opt ULARGE_INTEGER * pos,
	__in LARGE_INTEGER const distanceToMove,
	__in const FsStreamSeek MoveMethod)
{
	if (m_hFile == INVALID_HANDLE_VALUE) return E_NOT_SET;
	DWORD dwMoveMethod = 0;
	switch (MoveMethod)
	{
	case IFsStream::FsStreamBegin:
		dwMoveMethod = FILE_BEGIN;
		break;

	case IFsStream::FsStreamCurrent:
		dwMoveMethod = FILE_CURRENT;
		break;

	case IFsStream::FsStreamEnd:
		dwMoveMethod = FILE_END;
		break;

	default:
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;
	if (FALSE == SetFilePointerEx(m_hFile, distanceToMove, (PLARGE_INTEGER)&m_currentPos, dwMoveMethod))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		if (pos) *pos = m_currentPos;
		if (m_cachePos.QuadPart > m_currentPos.QuadPart ||
			m_cachePos.QuadPart + m_cacheSize < m_currentPos.QuadPart)
		{
			m_cacheSize = 0;
		}
	}
	return hr;
}

void WINAPI CFileFsStream::SetFileHandle(__in void* const handle)
{
	m_hFile = (HANDLE)handle;
	if (m_hFile != NULL && m_hFile != INVALID_HANDLE_VALUE)
	{
		// Init cache
		SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);
		DWORD r;
		if (ReadFile(m_hFile, m_cache, DEFAULT_MAX_CACHE_SIZE, &r, NULL) && r > 0)
		{
			m_cacheSize = (size_t)r;
		}
		ZeroMemory(&m_cachePos, sizeof(m_cachePos));
		SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);
	}
}

HRESULT WINAPI CFileFsStream::Shrink(void)
{
	if (m_hFile == NULL || m_hFile == INVALID_HANDLE_VALUE) return E_NOT_VALID_STATE;
	if (!SetEndOfFile(m_hFile))
		return HRESULT_FROM_WIN32(GetLastError());

	if ((m_cachePos.QuadPart <= m_currentPos.QuadPart) &&
		(m_currentPos.QuadPart < m_cachePos.QuadPart + m_cacheSize))
		m_cacheSize = (size_t)(m_currentPos.QuadPart - m_cachePos.QuadPart);
	else
		m_cacheSize = 0;
	return S_OK;
}
