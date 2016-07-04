#include "ZipFsAttribute.h"
#include "../../Utils.h"

CZipFsAttribute::CZipFsAttribute(void)
{
}

CZipFsAttribute::~CZipFsAttribute(void)
{
}

HRESULT WINAPI CZipFsAttribute::QueryAttributes(void)
{
	char filename_inzip[256];
	unz_file_info64 file_info;
	if (m_handle == NULL) return E_NOT_SET;

	int err = unzGetCurrentFileInfo64((unzFile)m_handle, &file_info,
		filename_inzip, sizeof(filename_inzip),
		NULL, 0, NULL, 0);
	if (err != UNZ_OK)
		return E_FAIL;

	// file size
	m_wfd.nFileSizeHigh = file_info.uncompressed_size >> 32;
	m_wfd.nFileSizeLow = file_info.uncompressed_size & 0xFFFFFFFF;

	// attribute
	m_wfd.dwFileAttributes = file_info.external_fa;

	// time
	DosDateTimeToFileTime(file_info.dosDate >> 16, (WORD)file_info.dosDate, &m_wfd.ftLastWriteTime);
	m_wfd.ftCreationTime = m_wfd.ftLastWriteTime;
	m_wfd.ftLastAccessTime = m_wfd.ftLastWriteTime;

	// file name
	StringA strNameA = filename_inzip;
	StringW strNameW = AnsiToUnicode(&strNameA);
	wcscpy_s(m_wfd.cFileName, MAX_PATH, strNameW.c_str());
	m_bInited = TRUE;
	return S_OK;
}

HRESULT WINAPI CZipFsAttribute::SetTime(__in_opt FILETIME *lpCreationTime, __in_opt FILETIME *lpLastAccessTime, __in_opt FILETIME *lpLastWriteTime)
{
	UNREFERENCED_PARAMETER(lpCreationTime);
	UNREFERENCED_PARAMETER(lpLastWriteTime);
	UNREFERENCED_PARAMETER(lpLastAccessTime);

	return E_NOTIMPL;
}

HRESULT WINAPI CZipFsAttribute::SetFilePath(__in LPCWSTR lpFilePath, __in_opt void* handle /*= NULL*/)
{
	m_handle = (HANDLE)handle;
	m_fileName = lpFilePath;
	return QueryAttributes();
}

