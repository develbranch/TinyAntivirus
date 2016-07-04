#pragma once
#include <TinyAvCore.h>
#include <vector>
#include <map>

class CScanService;

typedef struct SCAN_THREAD_PARAM {
	HANDLE threadHandle;
	HANDLE stopEvent;
	IFsEnumContext *enumContext;
	IFsEnum * enumurate;
	CScanService * instance;
}SCAN_THREAD_PARAM;

typedef std::map<IFsEnumContext *, SCAN_THREAD_PARAM*> SCAN_CONTEXT_MAP;

class CScanService:
	public CRefCount, 
	public IScanner,
	public IFsEnumObserver, 
	public IScanObserver
{
protected:
	std::vector<IScanObserver *> m_Observers;
	std::vector<IScanModule *> m_ScanModules;

	virtual ~CScanService();

public:
	CScanService();
	
	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);

	// IScanner interface implementation
	virtual HRESULT WINAPI AddScanObserver(__in IScanObserver *observer) override;

	virtual HRESULT WINAPI RemoveScanObserver(__in IScanObserver *observer) override;

	virtual HRESULT WINAPI AddScanModule(__in IScanModule *scanModule) override;

	virtual HRESULT WINAPI RemoveScanModule(__in IScanModule *scanModule) override;

	virtual HRESULT WINAPI Start(__in IFsEnumContext *enumContext) override;

	virtual HRESULT WINAPI Stop(__in IFsEnumContext *enumContext) override;

	virtual HRESULT WINAPI Pause(__in IFsEnumContext *enumContext) override;

	virtual HRESULT WINAPI Resume(__in IFsEnumContext *enumContext) override;

	// IFsEnumObserver interface implementation
	virtual HRESULT WINAPI OnFileFound(__in IVirtualFs *file, __in IFsEnumContext *context, __in const int currentDepth) override;

	// IScanObserver interface implementation
	virtual HRESULT WINAPI OnScanStarted(__in IFsEnumContext * context) override;


	virtual HRESULT WINAPI OnScanPaused(__in IFsEnumContext * context) override;


	virtual HRESULT WINAPI OnScanResumed(__in IFsEnumContext * context) override;


	virtual HRESULT WINAPI OnScanStopping(__in IFsEnumContext * context) override;


	virtual HRESULT WINAPI OnPreScan(__in IVirtualFs * file, __in IFsEnumContext * context) override;

	virtual HRESULT WINAPI OnAllScanFinished(__in IVirtualFs * file, __in IFsEnumContext * context) override;

	virtual HRESULT WINAPI OnPreClean(__in IVirtualFs * file, __in IFsEnumContext * context, __inout SCAN_RESULT * result) override;

	virtual HRESULT WINAPI OnPostClean(__in IVirtualFs * file, __in IFsEnumContext * context, __in SCAN_RESULT * result) override;
	
	virtual void WINAPI OnError(__in DWORD dwErrorCode, __in_opt LPCWSTR lpMessage = NULL) override;

	virtual void WINAPI Forever(void) override;


private:
	static DWORD WINAPI ScanThread(__in LPVOID lpParam);
public:
	static SCAN_CONTEXT_MAP m_ContextMap;
protected:
	virtual void WINAPI OnScanThread(__in SCAN_THREAD_PARAM * param);
	virtual void WINAPI AddArchivers(__inout IFsEnum * enumurate);
};