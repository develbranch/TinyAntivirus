#include "BufferedStream.h"

CBufferedStream::CBufferedStream(void) :
	m_FileSize(0),
	m_CurrPos(0)
{
}

CBufferedStream::~CBufferedStream(void)
{
}

HRESULT WINAPI CBufferedStream::QueryInterface(
	__in REFIID riid,
	__out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(IFsStream)))
	{
		*ppvObject = static_cast<IFsStream*>(this);
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT WINAPI CBufferedStream::Read(__out_bcount(bufferSize) LPVOID buffer, __in ULONG bufferSize, __out_opt ULONG * readSize)
{
	if (buffer == NULL || bufferSize == 0) return E_INVALIDARG;
	ULONGLONG copySize;

	if (m_CurrPos + bufferSize > m_FileSize)
		copySize = (ULONGLONG)(m_FileSize - m_CurrPos);
	else
		copySize = bufferSize;

	if (readSize) *readSize = (ULONG)copySize;
	if (copySize == 0) return E_NOT_VALID_STATE;

	memcpy(buffer, &m_DataStream[(size_t)m_CurrPos], (size_t)copySize);
	m_CurrPos += copySize;

	return S_OK;
}

HRESULT WINAPI CBufferedStream::ReadAt(
	__in LARGE_INTEGER const offset, __in const FsStreamSeek moveMethod, 
	__out_bcount(bufferSize) LPVOID buffer, __in ULONG bufferSize, __out_opt ULONG * readSize)
{
	HRESULT hr = Seek(NULL, offset, moveMethod);
	if (FAILED(hr)) return hr;
	return Read(buffer, bufferSize, readSize);
}

HRESULT WINAPI CBufferedStream::Write(__in_bcount(bufferSize) LPCVOID buffer, __in ULONG bufferSize, __out_opt ULONG * writtenSize)
{
	if (buffer == NULL || bufferSize == 0) return E_INVALIDARG;

	if (m_CurrPos == m_FileSize)
	{
		m_DataStream.insert(m_DataStream.end(), bufferSize, 0);
		memcpy(&m_DataStream[(size_t)m_CurrPos], buffer, bufferSize);
		m_FileSize = m_CurrPos = m_CurrPos + (ULONGLONG)bufferSize;
		if (*writtenSize) *writtenSize = bufferSize;
		return S_OK;
	}
	else if (m_CurrPos < m_FileSize)
	{
		if ((m_CurrPos + (ULONGLONG)bufferSize) <= m_FileSize)
		{
			memcpy(&m_DataStream[(size_t)m_CurrPos], buffer, bufferSize);
			m_CurrPos += (ULONGLONG)bufferSize;
			if (*writtenSize) *writtenSize = bufferSize;
			return S_OK;
		}
		else
		{
			m_DataStream.insert(m_DataStream.end(), (size_t)(m_CurrPos + (ULONGLONG)bufferSize - m_FileSize), 0);
			memcpy(&m_DataStream[(size_t)m_CurrPos], buffer, bufferSize);
			m_FileSize = m_CurrPos = m_CurrPos + (ULONGLONG)bufferSize;
			if (*writtenSize) *writtenSize = bufferSize;
			return S_OK;
		}
	}

	return E_NOT_VALID_STATE;
}

HRESULT WINAPI CBufferedStream::WriteAt(
	__in LARGE_INTEGER const offset, __in const FsStreamSeek moveMethod, 
	__in_bcount(bufferSize) LPCVOID buffer, __in ULONG bufferSize, __out_opt ULONG * writtenSize)
{
	HRESULT hr = Seek(NULL, offset, moveMethod);
	if (FAILED(hr)) return hr;
	return Write(buffer, bufferSize, writtenSize);
}

HRESULT WINAPI CBufferedStream::Tell(__out ULARGE_INTEGER * pos)
{
	if (pos == NULL) return E_INVALIDARG;

	pos->QuadPart = m_CurrPos;
	return S_OK;
}

HRESULT WINAPI CBufferedStream::Seek(__out_opt ULARGE_INTEGER * pos, __in LARGE_INTEGER const distanceToMove, __in const FsStreamSeek MoveMethod)
{
	ULONGLONG newPos;
	if (pos == NULL) return E_INVALIDARG;

	switch (MoveMethod)
	{
	case FsStreamBegin:
		newPos = distanceToMove.QuadPart;
		break;

	case FsStreamCurrent:
		newPos = m_CurrPos + distanceToMove.QuadPart;
		break;

	case FsStreamEnd:
		newPos = m_FileSize + distanceToMove.QuadPart;
		break;

	default:
		return E_INVALIDARG;
	}

	if (newPos > m_FileSize) return E_INVALIDARG;
	m_CurrPos = newPos;
	if (pos)
		pos->QuadPart = m_CurrPos;
	return S_OK;
}

void WINAPI CBufferedStream::SetFileHandle(__in void* const handle)
{
	if ((HANDLE)handle == INVALID_HANDLE_VALUE || handle == NULL)
	{
		m_DataStream.clear();
		m_FileSize = m_CurrPos = 0;
	}
}

HRESULT WINAPI CBufferedStream::Shrink(void)
{
	m_DataStream.resize((size_t)m_CurrPos);
	m_FileSize = m_DataStream.size();
	m_CurrPos--;
	return S_OK;
}
