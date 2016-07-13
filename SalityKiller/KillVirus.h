#pragma once
#include <InitGuid.h>
#include <TinyAvCore.h>

class CKillVirus :
	public CRefCount,
	public IScanModule,
	public IEmulObserver
{
private:
	uc_hook m_hookcode;
	uc_hook m_hookmem;

protected:
	BYTE*       m_OepCode;
	DWORD       m_salityEp;
	DWORD       m_dwOepCodeSize;
	DWORD	    m_OepAddr;
	ULONGLONG   m_InsCount;
	SCAN_RESULT m_scanResult;
	MODULE_INFO m_info;
	IPeFile *   m_parser;
	IEmulator * m_emul;
	DWORD		m_emulErrCode;
	virtual ~CKillVirus();

public:
	CKillVirus();

	// Implementing IUnknown interface
	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __out void **ppvObject);

	// Implementing IModule interface
	virtual HRESULT WINAPI GetModuleInfo(__out MODULE_INFO * scanInfo) override;

	virtual ModuleType WINAPI GetType(void) override;

	virtual HRESULT WINAPI GetName(__out BSTR *name) override;

	// Implementing IScanModule interface
	virtual HRESULT WINAPI OnScanInitialize(void) override;

	virtual HRESULT WINAPI Scan(__in IVirtualFs * file, __in IFsEnumContext * context, __in IScanObserver * observer) override;

	virtual HRESULT WINAPI OnScanShutdown(void) override;

protected:
	// Implementing IEmulObserver interface 
	virtual HRESULT WINAPI OnEmulatorStarting(void) override;
	virtual void WINAPI OnError(__in DWORD const dwErrorCode) override;
	virtual HRESULT WINAPI OnEmulatorStopped(void) override;

	virtual void OnHookCode(uint64_t address, uint32_t size);
	//
	virtual BOOL VerifySignature(__in_bcount(size) LPBYTE buffer, __in DWORD const size);

private:
	static void HookCode(uc_engine *uc, uint64_t address, uint32_t size, void *user_data);
	static bool HookMemInvalid(uc_engine *uc, uc_mem_type type,
		uint64_t address, int size, int64_t value, void *user_data);
};