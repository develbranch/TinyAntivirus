#pragma once
#include <windows.h>
int Kmp(__in_ecount(cbTargetSize) unsigned char *lpTarget, __in size_t cbTargetSize,
	__in_ecount(cbPatternSize) unsigned char *lpPattern, __in size_t cbPatternSize);