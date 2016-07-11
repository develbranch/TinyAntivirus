#pragma once
#include <TinyAvCore.h>

class CFileFsAttribute:
	public CRefCount,
	public IFsAttribute
{
protected:
	virtual ~CFileFsAttribute();
	StringW m_fileName;
	HANDLE  m_handle;
	WIN32_FIND_DATAW m_wfd;
	BOOL m_bInited;
public:
	CFileFsAttribute();

	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual HRESULT WINAPI Size(__out ULARGE_INTEGER * fileSize) override;

	virtual HRESULT WINAPI Attributes(__out DWORD *attribs) override;

	virtual HRESULT WINAPI SetAttributes(__in DWORD attribs) override;

	virtual HRESULT WINAPI Time(__out_opt FILETIME *lpCreationTime, __out_opt FILETIME *lpLastAccessTime, __out_opt FILETIME *lpLastWriteTime) override;

	virtual HRESULT WINAPI SetTime(__in_opt FILETIME *lpCreationTime, __in_opt FILETIME *lpLastAccessTime, __in_opt FILETIME *lpLastWriteTime) override;

	virtual HRESULT WINAPI SetFilePath(__in LPCWSTR lpFilePath, __in_opt void* handle /*= NULL*/) override;

protected:
	virtual HRESULT WINAPI QueryAttributes(void);

};

