#pragma once
#include "../TinyAvBase.h"

enum ModuleType
{
	DefaultModuleType = 1,
	ScanModule
};

typedef struct MODULE_INFO {
	ModuleType type;
	WCHAR      name[MAX_NAME + 1];
	HMODULE	   handle;
}MODULE_INFO, *LPMODULE_INFO;

MIDL_INTERFACE("151BBAB1-5D35-4A40-9940-09C08A412B89")
IModule : public IUnknown
{
public:
	BEGIN_INTERFACE

	/* Retrieve module information
	@scanInfo: a pointer to an variable storing MODULE_INFO
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI GetModuleInfo(__out MODULE_INFO * scanInfo) = 0;
	
	/* Retrieve module information
	@return: module type
	*/
	virtual ModuleType WINAPI GetType(void) = 0;
	
	/* Retrieve module name
	@name: a pointer to an variable storing name
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI GetName(__out BSTR *name) = 0;

	END_INTERFACE
};

/*
All plug-in module MUST have .plg extension and export a function named CreateModuleObject.
Exported function MUST have prototype that is declared as follows:

HRESULT (WINAPI *CREATEMODULEOBJECT)(__in REFCLSID rclsid, __in DWORD dwClsContext, __in REFIID riid, __out LPVOID *ppv);

@rclsid: The CLSID associated with the data and code that will be used to create the object.
@dwClsContext: Context in which the code that manages the newly created object will run.
@riid: A reference to the identifier of the interface to be used to communicate with the object.
@ppv: Address of pointer variable that receives the interface pointer requested in riid.
Upon successful return, *ppv contains the requested interface pointer. Upon failure, *ppv contains NULL.

@return: HRESULT on success, or other value on failure.
*/
#define MODULE_EXTENSION L"plg"
#define MODULE_EP ("CreateModuleObject")
typedef HRESULT (WINAPI *CREATEMODULEOBJECT)(__in REFCLSID rclsid, __in DWORD dwClsContext, __in REFIID riid, __out LPVOID *ppv);
