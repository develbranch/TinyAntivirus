#pragma once
#include "../TinyAvBase.h"
#include "Module.h"

MIDL_INTERFACE("19D1F804-DDBC-41CC-922A-DE4B177D5482")
IModuleManager : public IUnknown
{
public:
	BEGIN_INTERFACE

	/*Load plug-in modules by directory, name and flag
	@lpModuleDirectory: The directory to be used to load modules. 
		If this parameter is NULL, current directory is used.
	@lpModuleName: module name.
		If this parameter is NULL, all modules are loaded.
	@flags: The action to be taken when loading the module.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Load(__in LPCWSTR lpModuleDirectory = NULL, __in LPCWSTR lpModuleName = NULL, __in DWORD flags = 0) = 0;

	/*Load plug-in modules by directory, type and flag
	@lpModuleDirectory: The directory to be used to load modules.
	If this parameter is NULL, current directory is used.
	@moduleType: module type. If this parameter is DefaultModuleType, all modules are loaded.
	@flags: The action to be taken when loading the module.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Load(__in LPCWSTR lpModuleDirectory = NULL, __in ModuleType moduleType = DefaultModuleType, __in DWORD flags = 0) = 0;
	/*Unload plug-in modules by directory, name and flag
	@lpModuleName: module name. If this parameter is NULL, all modules are unloaded.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Unload(__in LPCWSTR lpModuleName = NULL) = 0;
	/*Unload plug-in modules by directory, type and flag
	@moduleType: module type.  If this parameter is DefaultModuleType, all modules are unloaded.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Unload(__in ModuleType moduleType = DefaultModuleType) = 0;

	/*Retrieve modules by their names
	@module: a pointer to a variable storing IModule* array 
	@moduleCount: reference to number of module.
	@lpModuleName: module name. If this parameter is NULL, all modules are used.
	@return: HRESULT on success, or other value on failure.
	*/

	virtual HRESULT WINAPI QueryModule(__out IModule **&module, __out size_t& moduleCount, __in LPCWSTR lpModuleName = NULL) = 0;
	/*Retrieve modules by their types
	@module: a pointer to a variable storing IModule* array
	@moduleCount: reference to number of module.
	@moduleType: module type. If this parameter is DefaultModuleType, all modules are used.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI QueryModule(__out IModule **&module, __out size_t& moduleCount, __in ModuleType moduleType = DefaultModuleType) = 0;
	END_INTERFACE
};
