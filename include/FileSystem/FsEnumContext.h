#pragma once
#include "../TinyAvBase.h"
#include "FsObject.h"

MIDL_INTERFACE("B417E769-E4A4-4D99-BF5D-3B3C2514394A")
IFsEnumContext: public IUnknown
{

public:
	enum EnumContextFlags
	{
		DetectOnly = 1,
		Disinfect  = 2
	};

	BEGIN_INTERFACE

	/*Set search container
	@container: a pointer to IFsObject that describes search area.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI SetSearchContainer(__in IVirtualFs * container) = 0;
	
	/*Retrieve search container
	@container:  pointer to a variable storing result.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI GetSearchContainer(__out IVirtualFs **container) = 0;
	
	/*Set search pattern
	@searchPattern: a pointer to a variable storing search pattern.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI SetSearchPattern(__in LPCWSTR searchPattern) = 0;
	
	/*Retrieve search pattern
	@searchPattern: a pointer to a variable storing result.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI GetSearchPattern(__out BSTR *searchPattern) = 0;
	
	// Set search depth
	virtual HRESULT WINAPI SetMaxDepth(__in const int maxDepth) = 0;
	virtual HRESULT WINAPI SetDepth(__in const int depth) = 0;
	virtual HRESULT WINAPI SetMaxDepthInArchive(__in const int maxDepth) = 0;
	virtual HRESULT WINAPI SetDepthInArchive(__in const int depth) = 0;
	
	// Retrieve max depth
	// @return: max depth
	virtual int WINAPI GetMaxDepth(void) = 0;

	// Retrieve depth
	// @return: current depth
	virtual int WINAPI GetDepth(void) = 0;

	// Retrieve max depth in archive
	// @return: max depth
	virtual int WINAPI GetMaxDepthInArchive(void) = 0;
	
	// Retrieve current depth in archive
	// @return: current depth
	virtual int WINAPI GetDepthInArchive(void) = 0;

	// set max file size
	// @fileSize: max file size
	// @return: HRESULT on success, or other value on failure.
	virtual HRESULT WINAPI SetMaxFileSize(__in ULARGE_INTEGER fileSize) = 0;
	
	// Retrieve max file size
	// @fileSize: a pointer to a variable storing result.
	// @return: HRESULT on success, or other value on failure.
	virtual HRESULT WINAPI GetMaxFileSize(__in ULARGE_INTEGER *fileSize) = 0;

	virtual HRESULT WINAPI SetFlags(__in const ULONG flags) = 0;
	virtual ULONG WINAPI GetFlags( void ) = 0;

	virtual HRESULT WINAPI AddIgnoreItem(__in LPCWSTR lpPath) = 0;
	virtual HRESULT WINAPI RemoveIgnoreItem(__in LPCWSTR lpPath) = 0;
	virtual HRESULT WINAPI GetIgnoreList(__out BSTR* lpPath, __out UINT *itemCount) = 0;
	virtual HRESULT WINAPI FreeIgnoreList(__in BSTR* lpPath, __in UINT itemCount) = 0;
	
	END_INTERFACE
};