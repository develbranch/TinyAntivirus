#include "ZipFs.h"
#include "../../Utils.h"
#include "../BufferedStream.h"
#include "ZipFsAttribute.h"

CZipFs::CZipFs()
{
	m_fsType = IVirtualFs::archive;
	ZeroMemory(&m_currentFilePos, sizeof(m_currentFilePos));
	if (m_attribute)m_attribute->Release();
	m_attribute = static_cast<IFsAttribute*> (new CZipFsAttribute());
	if (m_stream)m_stream->Release();
	m_stream = static_cast<IFsStream *> (new CBufferedStream());
	m_delimiter = StringW(L">");
}

CZipFs::~CZipFs()
{
	Close();
}

HRESULT WINAPI CZipFs::Create(__in LPCWSTR lpFileName, __in ULONG const flags)
{
	if (m_container == NULL)
		return E_NOT_SET;
	m_FileName = lpFileName;
	m_flags = flags;

	if (m_attribute == NULL)
	{
		Close();
		return E_OUTOFMEMORY;
	}
	m_attribute->SetFilePath(m_FileName.c_str());
	return S_OK;
}

HRESULT WINAPI CZipFs::Close(void)
{
	HRESULT hr = S_OK;
	if (m_handle != INVALID_HANDLE_VALUE && m_handle != NULL)
	{
		hr = (unzCloseCurrentFile((void*)m_handle) == UNZ_OK) ? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			if (UNZ_OK != unzGoToFilePos64((void*)m_handle, &m_currentFilePos))
				hr = E_UNEXPECTED;
		}
	}
	
	m_handle = INVALID_HANDLE_VALUE;
	m_stream->SetFileHandle(m_handle);
	return hr;
}

HRESULT WINAPI CZipFs::ReCreate(__in_opt void* handle, __in_opt ULONG const flags /*= 0*/)
{
	if (flags)
		m_flags = flags;

	if ((HANDLE)handle == m_handle)
		return S_OK;

	m_handle = (HANDLE)handle;

	StringA strNameA = UnicodeToAnsi(m_FileName);

	if (UNZ_OK != unzGetFilePos64((unzFile)m_handle, &m_currentFilePos))
		return E_NOT_SET;

	if (unzLocateFile((unzFile)m_handle, strNameA.c_str(), 0) != UNZ_OK)
		return E_FAIL;

	HRESULT hr = (unzOpenCurrentFile((unzFile)m_handle) == UNZ_OK) ? S_OK : E_FAIL;
	if (FAILED(hr))
	{
		m_handle = INVALID_HANDLE_VALUE;
		return hr;
	}

	if (m_stream)
	{
		m_stream->Release();
		m_stream = NULL;
	}
	m_stream = new CBufferedStream();
	if (m_stream == NULL)
	{
		Close();
		return E_OUTOFMEMORY;
	}

	int err = 0;
	unsigned char *pTemp = new unsigned char[WRITEBUFFERSIZE];
	if (pTemp)
	{
		do
		{
			err = unzReadCurrentFile((unzFile)m_handle, pTemp, WRITEBUFFERSIZE);
			if (err < 0)
			{
				break;
			}
			if (err > 0)
			{
				ULONG writtenSize;
				if (FAILED(m_stream->Write(pTemp, (ULONG)err, &writtenSize)))
					break;
				if (writtenSize == 0)
					break;
			}
		} while (err > 0);
		delete[] pTemp;
	}

	// goto the beginning of file
	ULARGE_INTEGER pos = {};
	LARGE_INTEGER distanceToMove = {};
	m_stream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin);
	m_attribute->SetFilePath(m_FileName.c_str(), handle);
	return S_OK;
}
