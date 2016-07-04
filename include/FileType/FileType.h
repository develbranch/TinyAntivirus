#pragma once
#include "../TinyAvBase.h"
#include "../FileSystem/FsObject.h"
// -----------------------------
// Interface for File parser
// -----------------------------

MIDL_INTERFACE("2C5B9040-59F9-4ACE-B201-45A8F02851E4")
IFileType : public IUnknown
{
public:

	BEGIN_INTERFACE

	// Check for type matching
	// @fsFile: a pointer to IFsObject object
	// @typeMatched: a pointer to a variable storing result.
	//@return: HRESULT on success, or other value on failure.
	virtual HRESULT WINAPI CheckType(__in IVirtualFs* fsFile, __out BOOL *typeMatched) = 0;
	
	END_INTERFACE
};