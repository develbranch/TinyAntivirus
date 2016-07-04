#pragma once
#include "TinyAvBase.h"
#include "Module/Module.h"
#include "Scanner/ScanModule.h"
#include "FileType/PEFile.h"
#include "Emulator/Emulator.h"
#include "Module/ModuleManager.h"
#include "Scanner/Scanner.h"
#include "FileSystem/FsObject.h"
#include "FileSystem/FsEnum.h"
#include <unicorn/unicorn.h>

#ifdef __cplusplus
extern "C"
{
#endif

	/*
	Create a class object that refers to the specified CLSID 
	
	@rclsid: The CLSID associated with the data and code that will be used to create the object.
	@dwClsContext: Context in which the code that manages the newly created object will run. 
	@riid: A reference to the identifier of the interface to be used to communicate with the object.
	@ppv: Address of pointer variable that receives the interface pointer requested in riid. 
		Upon successful return, *ppv contains the requested interface pointer. Upon failure, *ppv contains NULL.

	@return: HRESULT on success, or other value on failure.
	*/
	HRESULT WINAPI CreateClassObject(__in REFCLSID rclsid, __in DWORD dwClsContext, __in REFIID riid, __out LPVOID *ppv);

#ifdef __cplusplus
}
#endif

// class UUID

// {84BD7A7C-720C-4A3C-A7DD-6DAA90375F7B}
DEFINE_GUID(CLSID_CPeFileParser,
	0x84bd7a7c, 0x720c, 0x4a3c, 0xa7, 0xdd, 0x6d, 0xaa, 0x90, 0x37, 0x5f, 0x7b);

// {B714C028-FE14-4CAA-81AC-7954E96EE3D2}
DEFINE_GUID(CLSID_CPeEmulator,
	0xb714c028, 0xfe14, 0x4caa, 0x81, 0xac, 0x79, 0x54, 0xe9, 0x6e, 0xe3, 0xd2);

// {507A919C-9161-469F-8ACE-EFC3B886B6FC}
DEFINE_GUID(CLSID_CModuleMgrService,
	0x507a919c, 0x9161, 0x469f, 0x8a, 0xce, 0xef, 0xc3, 0xb8, 0x86, 0xb6, 0xfc);

// {FCFAE438-4304-4FF9-8EEA-BF29D03AB5AC}
DEFINE_GUID(CLSID_CScanService,
	0xfcfae438, 0x4304, 0x4ff9, 0x8e, 0xea, 0xbf, 0x29, 0xd0, 0x3a, 0xb5, 0xac);

// {BE54543C-AB95-411E-A8B6-6EB0E4E4E950}
DEFINE_GUID(CLSID_CFileFsEnumContext,
	0xbe54543c, 0xab95, 0x411e, 0xa8, 0xb6, 0x6e, 0xb0, 0xe4, 0xe4, 0xe9, 0x50);

// {2928278F-CE4E-4263-9F8C-07089796643C}
DEFINE_GUID(CLSID_CFileFs,
	0x2928278f, 0xce4e, 0x4263, 0x9f, 0x8c, 0x7, 0x8, 0x97, 0x96, 0x64, 0x3c);

