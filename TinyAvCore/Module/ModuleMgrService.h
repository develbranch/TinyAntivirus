#pragma once
#include <TinyAvCore.h>

typedef std::vector<IModule*> MODULE_ARRAY;

class CModuleMgrService :
	public CRefCount,
	public IModuleManager
{
protected:
	MODULE_ARRAY m_modules;

public:
	CModuleMgrService();
	virtual ~CModuleMgrService();

	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(
		__in REFIID riid,
		_COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual HRESULT WINAPI Load(__in LPCWSTR lpModuleDirectory = NULL, __in LPCWSTR lpModuleName = NULL, __in DWORD flags = 0) override;


	virtual HRESULT WINAPI Load(__in LPCWSTR lpModuleDirectory = NULL, __in ModuleType moduleType = DefaultModuleType, __in DWORD flags = 0) override;


	virtual HRESULT WINAPI Unload(__in LPCWSTR lpModuleName = NULL) override;


	virtual HRESULT WINAPI Unload(__in ModuleType moduleType = DefaultModuleType) override;


	virtual HRESULT WINAPI QueryModule(__out IModule **&module, __out size_t& moduleCount, __in LPCWSTR lpModuleName = NULL) override;


	virtual HRESULT WINAPI QueryModule(__out IModule **&module, __out size_t& moduleCount, __in ModuleType moduleType = DefaultModuleType) override;

};
