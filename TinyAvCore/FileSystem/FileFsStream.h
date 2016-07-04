#pragma once
#include <TinyAvCore.h>

#ifndef DEFAULT_MAX_CACHE_SIZE
#define DEFAULT_MAX_CACHE_SIZE (16 * 1024)
#endif

class CFileFsStream :
	public CRefCount,
	public IFsStream
{
protected:
	char *m_cache;
	size_t m_cacheSize;
	ULARGE_INTEGER m_cachePos;
	ULARGE_INTEGER m_currentPos;
	HANDLE m_hFile;

	virtual ~CFileFsStream();

public:
	CFileFsStream();

	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(
		__in REFIID riid,
		__out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual HRESULT WINAPI Read(__out LPVOID buffer, __in ULONG bufferSize, __out_opt ULONG * readSize) override;

	virtual HRESULT WINAPI Write(__in const void * buffer, __in ULONG bufferSize, __out_opt ULONG * writtenSize) override;

	virtual HRESULT WINAPI Tell(__out ULARGE_INTEGER * pos) override;

	virtual HRESULT WINAPI Seek(__out_opt ULARGE_INTEGER * pos, __in LARGE_INTEGER const distanceToMove, __in const FsStreamSeek MoveMethod) override;

	virtual void WINAPI SetFileHandle(__in void* const handle) override;

	virtual HRESULT WINAPI Shrink(void) override;

};
