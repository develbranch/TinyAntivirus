#include <windows.h>
#include <stdio.h>
#include <TinyAvCore.h>
#include <Shlwapi.h>
#if defined DEBUG || defined _DEBUG
#include <crtdbg.h>
#endif
#include "KillVirus.h"

HMODULE g_hMod = NULL;

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
#if defined DEBUG || defined _DEBUG
	{
		int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		_CrtSetDbgFlag(flag | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(0x1337);
	}
#endif

	UNREFERENCED_PARAMETER(lpReserved);

	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		g_hMod = hinstDLL;
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

#ifdef __cplusplus
extern "C"
{
#endif

	HRESULT WINAPI CreateModuleObject(__in REFCLSID rclsid, __in DWORD dwClsContext, __in REFIID riid, __out LPVOID *ppv)
	{
		UNREFERENCED_PARAMETER(rclsid);
		UNREFERENCED_PARAMETER(dwClsContext);
		if (ppv == NULL) return E_INVALIDARG;

		if (IsEqualIID(riid, __uuidof(IModule)))
		{
			*ppv = static_cast<IModule*>(new CKillVirus());
			return S_OK;
		}

		return E_NOINTERFACE;
	}

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////

// notelemetry
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
	void _cdecl __vcrt_initialize_telemetry_provider() {}
	void _cdecl __telemetry_main_invoke_trigger() {}
	void _cdecl __telemetry_main_return_trigger() {}
	void _cdecl __vcrt_uninitialize_telemetry_provider() {}
#ifdef __cplusplus
};
#endif // __cplusplus
//////////////////////////////////////////////////////////////////////////
