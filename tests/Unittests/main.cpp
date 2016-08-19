#include <gtest/gtest.h>
#pragma comment(lib, "gtest.lib" )
#pragma comment(lib, "TinyAvCore.lib" )
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib" )

#if defined DEBUG || defined _DEBUG
#include <crtdbg.h>
#endif

WCHAR szTestcase[MAX_PATH] = {};
WCHAR szSampleDir[MAX_PATH] = {};

int main(int argc, char** argv)
{
#if defined DEBUG || defined _DEBUG
	{
		int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		_CrtSetDbgFlag(flag | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(0x1337);
	}
#endif

	if (argc >= 2)
	{
		MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, szSampleDir, MAX_PATH);
	}
	else
	{
		return 1;
	}

	wcscpy_s(szTestcase, MAX_PATH, szSampleDir);
	PathAppendW(szTestcase, L"testcase.bin");
	
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}