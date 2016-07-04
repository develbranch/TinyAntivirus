#pragma once
#include "../TinyAvBase.h"
#include "FsObject.h"
#include "FsEnumContext.h"

MIDL_INTERFACE("D172D234-9C5B-4C37-949F-E05B089F8CE9")
IFsEnumObserver: public IUnknown
{
public:
	BEGIN_INTERFACE
	/*
	OnFileFound method is called when enumeration operation hits new file.
	@file: a pointer to IFsObject that describes new file.
	@context: a pointer to IFsEnumContext that describes enumeration context.
	@currentDepth: current depth.
	@return: the return value is S_OK if succeeds, or E_ABORT to stop enumeration.
	*/
	virtual HRESULT WINAPI OnFileFound(__in IVirtualFs *file, __in IFsEnumContext *context, __in const int currentDepth) = 0;

	//called when an error occurred
	// @dwErrorCode: error code
	// @lpMessage: Error message
	virtual void WINAPI OnError(__in DWORD dwErrorCode, __in_opt LPCWSTR lpMessage = NULL) = 0;
	END_INTERFACE
};