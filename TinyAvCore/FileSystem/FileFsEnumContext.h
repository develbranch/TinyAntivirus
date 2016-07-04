#pragma once
#include <TinyAvCore.h>
#include <vector>

#define MAX_FILE_SIZE (10 * 1024 * 1024)

class CFileFsEnumContext:
	public CRefCount, 
	public IFsEnumContext
{
protected:
	~CFileFsEnumContext();
	ULARGE_INTEGER m_maxSize;
	IVirtualFs *m_container;
	StringW m_searchPattern;
	std::vector<StringW> m_ignore;
	int		m_maxDepth;
	int		m_depth;
	int		m_maxArchiveDepth;
	int		m_ArchiveDepth;
	ULONG   m_flags;

public:
	CFileFsEnumContext();

	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual HRESULT WINAPI SetSearchContainer(__in IVirtualFs* container) override;

	virtual HRESULT WINAPI GetSearchContainer(__out IVirtualFs **container) override;

	virtual HRESULT WINAPI SetSearchPattern(__in LPCWSTR searchPattern) override;

	virtual HRESULT WINAPI GetSearchPattern(__out BSTR *searchPattern) override;

	virtual HRESULT WINAPI SetFlags(__in const ULONG flags) override;

	virtual ULONG WINAPI GetFlags(void) override;

	virtual  HRESULT WINAPI SetMaxDepth(__in const int maxDepth) override;
	virtual HRESULT WINAPI SetDepth(__in const int depth) override;

	virtual int WINAPI GetMaxDepth(void) override;

	virtual int WINAPI GetDepth(void) override;

	virtual HRESULT WINAPI SetMaxFileSize(__in ULARGE_INTEGER fileSize) override;

	virtual HRESULT WINAPI GetMaxFileSize(__in ULARGE_INTEGER *fileSize) override;

	virtual HRESULT WINAPI AddIgnoreItem(__in LPCWSTR lpPath) override;

	virtual HRESULT WINAPI RemoveIgnoreItem(__in LPCWSTR lpPath) override;

	virtual HRESULT WINAPI GetIgnoreList(__out BSTR* lpPath, __out UINT *itemCount) override;

	virtual HRESULT WINAPI FreeIgnoreList(__in BSTR* lpPath, __in UINT itemCount) override;


	virtual int WINAPI GetMaxDepthInArchive(void) override;


	virtual int WINAPI GetDepthInArchive(void) override;


	virtual HRESULT WINAPI SetMaxDepthInArchive(__in const int maxDepth) override;


	virtual HRESULT WINAPI SetDepthInArchive(__in const int depth) override;

};

