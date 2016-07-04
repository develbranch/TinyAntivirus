#include <windows.h>
#include <stdio.h>
#include <TinyAvCore.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include "getopt.h"
#include "ConsoleObserver.h"

#if defined DEBUG || defined _DEBUG
#include <crtdbg.h>
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


void Usage(void)
{
	puts("Read README.md for usage\n");
	exit(0);
}

void PrintWelcome()
{
	puts("------------------------------------------------------");
	puts("TinyAntivirus version 0.1");
	puts("Copyright (C) 2016, Quang Nguyen. All rights reserved.");
	puts("TinyAntivirus comes with ABSOLUTELY NO WARRANTY");
	puts("This is free software, and you are welcome to redistribute\n it under certain conditions");
	puts("Website: http://develbranch.com");
	puts("------------------------------------------------------");
}

int wmain(int argc, wchar_t* argv[])
{
	PrintWelcome();
#if defined DEBUG || defined _DEBUG
	{
		int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		_CrtSetDbgFlag(flag | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(0x1337);
	}
#endif
	HRESULT hr;
	WCHAR szPattern[MAX_PATH + 1] = L"*.*";
	WCHAR szTargetDir[MAX_PATH + 1] = {};
	WCHAR szPluginsSubDir[MAX_PATH + 1] = {};
	WCHAR szPluginsDir[MAX_PATH + 1] = {};
	int c;
	int depth = -1;
	int archiveDepth = -1;
	ULARGE_INTEGER maxFileSize = {};
	int mode = 2; //kill mode
	maxFileSize.QuadPart = 10 * 1024 * 1024;
	// -p
	while ((c = getopt_w(argc, argv, L"e:A:D:d:p:s:m:h")) != -1)
	{
		switch (c)
		{
		case L'e':
			wcscpy_s((wchar_t*)szPluginsSubDir, MAX_PATH, optarg_w);
			break;

		case L'A':
			archiveDepth = _wtoi(optarg_w);
			break;

		case L'D':
			depth = _wtoi(optarg_w);
			break;
		
		case L'd':
			wcscpy_s((wchar_t*)szTargetDir, MAX_PATH, optarg_w);
			break;

		case L'p':
			wcscpy_s((wchar_t*)szPattern, MAX_PATH, optarg_w);
			break;

		case L's':
			maxFileSize.QuadPart = _wtoi(optarg_w);
			break;
	
		case L'm': // mode
			if ((optarg_w[0] & 0xdf) == L'K') // kill mode
				mode = 2;
			if ((optarg_w[0] & 0xdf) == L'S') // Scan mode
				mode = 1;
			break;

		case L'h':
			Usage();
			break;
		default:
			Usage();
			break;
		}
	}

	if (wcslen(szTargetDir) == 0)
		return 1;

	IScanObserver * consoleObserver = NULL;
	IScanner * scanner = NULL;
	IModuleManager *mgr = NULL;
	IFsEnumContext * enumContext = NULL;
	IVirtualFs *container = NULL;

	if (FAILED(CreateClassObject(CLSID_CModuleMgrService, 0, __uuidof(IModuleManager), (LPVOID*)&mgr)) ||
		FAILED(CreateClassObject(CLSID_CScanService, 0, __uuidof(IScanner), (LPVOID*)&scanner)) ||
		FAILED(CreateClassObject(CLSID_CFileFsEnumContext, 0, __uuidof(IFsEnumContext), (LPVOID*)&enumContext)) ||
		FAILED(CreateClassObject(CLSID_CFileFs, 0, __uuidof(IVirtualFs), (LPVOID*)&container)) ||
		((consoleObserver = static_cast<IScanObserver *>(new CConsoleObserver)) == NULL)
		)
	{
		if (scanner) scanner->Release();
		if (mgr) mgr->Release();
		if (enumContext) enumContext->Release();
		if (container) container->Release();
		if (consoleObserver) consoleObserver->Release();
		return 1;
	}

	GetModuleFileNameW(NULL, szPluginsDir, MAX_PATH);
	PathRemoveFileSpecW(szPluginsDir);
	if (wcslen(szPluginsSubDir) > 0)
		PathAppendW(szPluginsDir, szPluginsSubDir);

	if (SUCCEEDED(mgr->Load(szPluginsDir, NULL, 0)))
	{
		IModule **scanModule = NULL;
		size_t moduleCount = 0;

		if (SUCCEEDED(mgr->QueryModule(scanModule, moduleCount, ScanModule)))
		{
			for (size_t i = 0; i < moduleCount; ++i)
			{
				scanner->AddScanModule(dynamic_cast<IScanModule*>(scanModule[i]));
				scanModule[i]->Release();
			}

			CoTaskMemFree(scanModule);
		}

		if (
			SUCCEEDED(hr = scanner->AddScanObserver(consoleObserver)) &&
			SUCCEEDED(hr = enumContext->SetSearchPattern(szPattern)) &&
			SUCCEEDED(hr = enumContext->SetMaxDepth(depth)) &&
			SUCCEEDED(hr = enumContext->SetMaxDepthInArchive(archiveDepth)) &&
			SUCCEEDED(hr = enumContext->SetMaxFileSize(maxFileSize)) &&
			SUCCEEDED(hr = enumContext->SetFlags((mode == 1) ? IFsEnumContext::DetectOnly : IFsEnumContext::Disinfect)) &&
			SUCCEEDED(hr = container->Create(szTargetDir, 0)) &&
			SUCCEEDED(hr = enumContext->SetSearchContainer(container))
			)
		{
			hr = scanner->Start(enumContext);
			scanner->Forever();
		}
	}
	consoleObserver->Release();
	enumContext->Release();
	container->Release();
	scanner->Release();
	mgr->Unload(ScanModule);
	mgr->Release();
	return 0;
}