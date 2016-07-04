#pragma once
#include <TinyAvCore.h>

class CFileFs: 
	public CRefCount, 
	public IVirtualFs
{
protected:
	StringW			m_delimiter;
	StringW			m_FileName;
	HANDLE			m_handle;
	ULONG			m_flags;
	ULONG			m_error;
	ULONG			m_fsType;
	IFsAttribute *	m_attribute;
	IFsStream *		m_stream;
	IVirtualFs *		m_container;

	virtual ~CFileFs();
public:
	CFileFs();

	// implementing IUnknown interface
	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);

	// implementing IFsObject interface
	virtual HRESULT WINAPI Create(__in LPCWSTR lpFileName, __in ULONG const flags) ;

	virtual HRESULT WINAPI Close(void) override;

	virtual HRESULT WINAPI ReCreate(__in_opt void* handle /*= NULL*/, __in_opt ULONG const flags /*= 0*/) override;

	virtual HRESULT WINAPI IsOpened(__out BOOL *isOpened) override;

	virtual HRESULT WINAPI GetFlags(__out ULONG *flags) override;

	virtual HRESULT WINAPI GetFullPath(__out BSTR *fullPath) override;

	virtual HRESULT WINAPI GetFileName(__out BSTR *fileName) override;

	virtual HRESULT WINAPI GetFileExt(__out BSTR *fileExt) override;

	virtual HRESULT WINAPI GetContainer(__out IVirtualFs **container) override;

	virtual HRESULT WINAPI SetContainer(__in IVirtualFs *container) override;

	virtual ULONG WINAPI GetError(void) override;

	virtual void WINAPI SetError(__in const ULONG error) override;

	virtual HRESULT WINAPI GetHandle(__out LPVOID * fileHandle) override;

	virtual HRESULT WINAPI GetFsType(__out ULONG * fsType) override;

	virtual HRESULT WINAPI DeferredDelete(void) override;

};