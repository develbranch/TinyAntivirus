#pragma once
#include "../TinyAvBase.h"
#include "FsEnumObserver.h"

MIDL_INTERFACE("A438AF0B-883B-4852-B261-DF2A882418AB")
IFsEnum : public IUnknown
{
	BEGIN_INTERFACE

public:
	enum FsEnumErrorCode
	{
		FsEnumErr = ENUMERATION_ERROR_CODE_BASE,
		FsEnumAccessDenied,
		FsEnumNotFound
	};

	/*
	Register an observer for enumeration
	The observer will be called when a new file is hit.

	@observer: a pointer to IFsEnumObserver object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI AddObserver(__in IFsEnumObserver *observer) = 0;
	
	/*
	Unregister (remove) an observer.
	This API removes the observer registered by AddObserver()

	@observer: a pointer to IFsEnumObserver object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI RemoveObserver(__in IFsEnumObserver *observer) = 0;

	/*
	Register an archiver for enumeration
	The observer will be called when a new file is hit.

	@archiver: a pointer to IFsEnum object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI AddArchiver(__in IFsEnum * archiver) = 0;

	/*
	Unregister (remove) an archiver.
	This API removes the observer registered by RemoveArchiver()

	@archiver: a pointer to IFsEnum object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI RemoveArchiver(__in IFsEnum * archiver) = 0;
	
	// Enumerate files and directories
	// @context: enumeration context
	// @return: HRESULT on success, or other value on failure.
	virtual HRESULT WINAPI Enum( __in IFsEnumContext *context) = 0;

	// Stop enumeration
	virtual void WINAPI Stop(void) = 0;

	END_INTERFACE
};
