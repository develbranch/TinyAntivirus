#pragma once
#include "../FileFsEnum.h"
#include <zlib/ioapi.h>

class CZipFsEnum :
	public CFileFsEnum
{
protected:

	virtual HRESULT WINAPI ReadArchiver(__in_opt IVirtualFs * container, __in IFsEnumContext * context, __in void * stream);
	virtual ~CZipFsEnum(void);

public:
	CZipFsEnum(void);
	
	virtual HRESULT WINAPI Enum(__in IFsEnumContext *context) override;
};