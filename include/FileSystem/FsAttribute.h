#pragma once
#include "../TinyAvBase.h"

MIDL_INTERFACE("4ADFFF42-170A-475E-AF09-C20C405309B5")
IFsAttribute : public IUnknown
{
	BEGIN_INTERFACE

public:
	
	/*
	Set file path and file handle
	@lpFilePath: a pointer to file path 
	@handle: file handle
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI SetFilePath(__in LPCWSTR lpFilePath, __in_opt void* handle = NULL) = 0;

	/*
	Retrieve the file size.
	@fileSize: a pointer a variable storing result.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Size(__out ULARGE_INTEGER * fileSize) = 0;

	/*
	Retrieve the file attribute.
	@attribs: a pointer a variable storing result.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Attributes(__out DWORD *attribs) = 0;

	/*
	Set file attribute.
	@attribs: file attribute.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI SetAttributes(__in DWORD attribs) = 0;

	/*
	Retrieve the date and time that a file .
	
	@lpCreationTime: A pointer to a FILETIME structure to receive the date and time the file was created.
	@lpLastAccessTime: A pointer to a FILETIME structure to receive the date and time the file was last accessed.
	@lpLastWriteTime: A pointer to a FILETIME structure to receive the date and time the file was last written to, truncated, or overwritten.
	
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Time(__out_opt FILETIME *lpCreationTime, __out_opt FILETIME *lpLastAccessTime, __out_opt FILETIME *lpLastWriteTime) = 0;

	/*
	Set the date and time that the specified file or directory was created, last accessed, or last modified.
	
	@lpCreationTime: A pointer to a FILETIME structure that contains the new creation date and time for the file.
	@lpLastAccessTime: A pointer to a FILETIME structure that contains the new last access date and time for the file.
	@lpLastWriteTime: A pointer to a FILETIME structure that contains the new last modified date and time for the file.
	
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI SetTime(__in_opt FILETIME *lpCreationTime, __in_opt FILETIME *lpLastAccessTime, __in_opt FILETIME *lpLastWriteTime) = 0;

	END_INTERFACE
};
