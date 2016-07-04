#pragma once
#include "../TinyAvBase.h"
#include "ScanModule.h"
#include "ScanObserver.h"

MIDL_INTERFACE("6BC6668B-E083-4FDA-9F27-EA4905BED319")
IScanner : public IUnknown
{
public:
	BEGIN_INTERFACE

	/* Add new scan observer to the monitor list
	@observer: a pointer to IScanObserver object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI AddScanObserver(__in IScanObserver *observer) = 0;

	/* Remove an scan observer from the monitor list
	@observer: a pointer to IScanObserver object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI RemoveScanObserver(__in IScanObserver *observer) = 0;

	/* Add scan module to engine
	@observer: a pointer to IScanObserver object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI AddScanModule(__in IScanModule *scanModule) = 0;

	/* Remove scan module from engine
	@observer: a pointer to IScanObserver object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI RemoveScanModule(__in IScanModule *scanModule) = 0;

	/* Start scanning
	@enumContext: a pointer to IFsEnumContext object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Start(__in IFsEnumContext *enumContext) = 0;

	/* Stop scanning
	@enumContext: a pointer to IFsEnumContext object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Stop(__in IFsEnumContext *enumContext) = 0;

	/* Pause scanning
	@enumContext: a pointer to IFsEnumContext object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Pause(__in IFsEnumContext *enumContext) = 0;

	/* Resume scanning
	@enumContext: a pointer to IFsEnumContext object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Resume(__in IFsEnumContext *enumContext) = 0;
	
	// wait for all threads to stop
	virtual void WINAPI Forever(void) = 0;
	
	END_INTERFACE
};