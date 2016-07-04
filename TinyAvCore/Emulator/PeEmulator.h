#pragma once
#include <TinyAvCore.h>

#ifndef DYNLOAD
#define DYNLOAD
#endif

// windows specific
#ifdef _MSC_VER
#include <io.h>
#include <windows.h>
#define PRIx64 "llx"
#ifdef DYNLOAD
#include "unicorn_dynload.h"
#else // DYNLOAD
#include <unicorn/unicorn.h>
#ifdef _WIN64
#pragma comment(lib, "unicorn_staload64.lib")
#else // _WIN64
#pragma comment(lib, "unicorn_staload.lib")
#endif // _WIN64
#endif // DYNLOAD

// posix specific
#else // _MSC_VER
#include <unistd.h>
#include <inttypes.h>
#include <unicorn/unicorn.h>
#endif // _MSC_VER
#include <vector>

class CPeEmulator 
	: public CRefCount
	, public IEmulator
{
protected:
	bool		m_starting;
	bool        m_bEmulatorEngineReady;
	uc_engine * m_engine;
	std::vector<IEmulObserver * > m_Observers;

private:
	HRESULT WINAPI OnStarting(void);
	void WINAPI    OnError(__in DWORD const dwErrorCode);
	HRESULT WINAPI OnStopped(void);

protected:
	virtual ~CPeEmulator();

public:
	CPeEmulator();

	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override;

	virtual HRESULT WINAPI ReadRegister(__in DWORD const reg, __out DWORD_PTR *regValue) override;

	virtual HRESULT WINAPI WriteRegister(__in DWORD const reg, __out DWORD_PTR const regValue) override;

	virtual HRESULT WINAPI ReadMemory(__in DWORD_PTR memoryAddr, __out_bcount(nNumberOfBytesToRead) LPVOID lpBuffer, __in DWORD nNumberOfBytesToRead) override;

	virtual HRESULT WINAPI WriteMemory(__in DWORD_PTR memoryAddr, __out_bcount(nNumberOfBytesToWrite) LPVOID lpBuffer, __in DWORD nNumberOfBytesToWrite) override;

	virtual HRESULT WINAPI EmulateCode(__in_bcount(nSizeOfCode) LPBYTE lpCodeBuffer, __in DWORD nSizeOfCode,
		__in DWORD_PTR memoryMappedAddr, __in DWORD nSizeOfStackCommit, __in DWORD nSizeOfStackReserve,
		__in DWORD_PTR addressToStart, __in DWORD nNumberOfBytesToEmulate) override;

	virtual HRESULT WINAPI EmulatePeFile(__in IPeFile *peFile, __in DWORD_PTR rvaToStart, __in int origin, __in DWORD nNumberOfBytesToEmulate = 0) override;

	virtual HRESULT WINAPI AddObserver(__in IEmulObserver *observer) override;

	virtual HRESULT WINAPI RemoveObserver(__in IEmulObserver *observer) override;

	virtual HRESULT __cdecl AddHook(__out void *hookHandle, __in int type, __in void * callback, __in void *user_data, ...) override;

	virtual HRESULT WINAPI RemoveHook(__in size_t hookHandle) override;

	virtual HRESULT WINAPI StopEmulator(void) override;

};