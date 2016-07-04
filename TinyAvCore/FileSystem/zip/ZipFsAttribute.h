#pragma once
#include "../FileFsAttribute.h"
#include <zlib/unzip.h>

class CZipFsAttribute :
	public CFileFsAttribute
{
protected:
	virtual ~CZipFsAttribute(void);

public:
	CZipFsAttribute(void);

	virtual HRESULT WINAPI QueryAttributes(void);
	
	virtual HRESULT WINAPI SetTime(__in_opt FILETIME *lpCreationTime, __in_opt FILETIME *lpLastAccessTime, __in_opt FILETIME *lpLastWriteTime) override;

	virtual HRESULT WINAPI SetFilePath(__in LPCWSTR lpFilePath, __in_opt void* handle /*= NULL*/) override;

};