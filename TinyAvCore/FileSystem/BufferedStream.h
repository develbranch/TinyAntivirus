#pragma once
#include <TinyAvCore.h>
#include <vector>

class CBufferedStream :
	public CRefCount,
	public IFsStream
{
protected:
	ULONGLONG			m_FileSize;
	ULONGLONG			m_CurrPos;
	std::vector<BYTE>	m_DataStream;
	virtual ~CBufferedStream(void);
public:
	CBufferedStream(void);
	
	DECLARE_REF_COUNT();
	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual HRESULT WINAPI Read(__out LPVOID buffer, __in ULONG bufferSize, __out_opt ULONG * readSize) override;

	virtual HRESULT WINAPI Write( __in const void * buffer, __in ULONG bufferSize, __out_opt ULONG * writtenSize ) override;

	virtual HRESULT WINAPI Tell(__out ULARGE_INTEGER * pos) override;

	virtual HRESULT WINAPI Seek(__out_opt ULARGE_INTEGER * pos, __in LARGE_INTEGER const distanceToMove, __in const FsStreamSeek MoveMethod) override;

	virtual void WINAPI SetFileHandle(__in void* const handle) override;

	virtual HRESULT WINAPI Shrink(void) override;

};
