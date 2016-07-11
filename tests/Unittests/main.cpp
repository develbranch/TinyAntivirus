#include <gtest/gtest.h>
#ifdef _DEBUG
#pragma comment(lib, "gtestd.lib" )
#endif // _DEBUG

#pragma comment(lib, "TinyAvCore.lib" )
#include <windows.h>
#include <shlwapi.h>
TCHAR szTestcase[MAX_PATH] = {};
#pragma comment(lib, "shlwapi.lib" )

#if defined DEBUG || defined _DEBUG
#include <crtdbg.h>
#endif

int main(int argc, char** argv)
{
#if defined DEBUG || defined _DEBUG
	{
		int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		_CrtSetDbgFlag(flag | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(0x1337);
	}
#endif

	GetModuleFileName(NULL, szTestcase, MAX_PATH);
	PathRemoveFileSpec(szTestcase);
	PathAppend(szTestcase, TEXT("testcase.bin"));
	
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}