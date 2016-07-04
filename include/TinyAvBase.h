#pragma once
#include <tchar.h>
#include <Windows.h>
#include <Unknwn.h>
#include <InitGuid.h>
#include <vector>
#include <stack>
#include <list>
#include "RefCount.h"

//////////////////////////////////////////////////////////////////////////
// String
#define StringA		std::string

#define StringW		std::wstring
#if defined _UNICODE || defined UNICODE
#define StringT		StringW
#else
#define StringT		StringA
#endif

//////////////////////////////////////////////////////////////////////////
#define TEST_FLAG(x,y) (((x) & (y)) == (y))
#define CLR_FLAG(x,f) ((x)=(x)&(~(f)))

//////////////////////////////////////////////////////////////////////////
#define MAX_NAME (256)

#define EMULATOR_ERROR_CODE_BASE	(100)
#define ENUMERATION_ERROR_CODE_BASE (200)