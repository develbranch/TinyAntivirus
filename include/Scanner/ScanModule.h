#pragma once
#include "../TinyAvBase.h"
#include "ScanObserver.h"
#include "../FileSystem/FsObject.h"
#include "../Module/Module.h"

MIDL_INTERFACE("7C84DAA3-D4E2-485F-8FE6-89C4419D4159")
IScanModule: public IModule
{
public:
	BEGIN_INTERFACE

	// Method is called when application initializes this module
	virtual HRESULT WINAPI OnScanInitialize(void) = 0;

	/* Scan file
	@file: a pointer to IFsObject object
	@context: a pointer to IFsEnumContext object
	@observer: a pointer to IScanObserver object
	@return: if function succeeds, the return value is S_OK.
	If file should be rescanned, the return value is S_FALSE.
	Otherwise, the return value is HRESULT error code.
	*/
	virtual HRESULT WINAPI Scan(__in IVirtualFs * file, __in IFsEnumContext * context, __in IScanObserver * observer) = 0;

	// Method is called when application shuts down this module
	virtual HRESULT WINAPI OnScanShutdown(void) = 0;

	END_INTERFACE
};