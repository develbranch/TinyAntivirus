#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

__checkReturn void* KmpAlloc(__in size_t cbSize)
{
	return malloc(cbSize);
}

void KmpFree(__in  void* p)
{
	free(p);
}

int *PrefixKmp(__in_ecount(size) unsigned char *pattern, __in size_t size)
{
	int k = -1;
	size_t i = 1;

	if (pattern == NULL || size == 0) return NULL;

	int *pi = (int *)KmpAlloc(sizeof(int)*size);
	if (!pi) return NULL;

	pi[0] = k;
	for (i = 1; i < size; ++i) {
		while (k > -1 && pattern[k + 1] != pattern[i])
			k = pi[k];
		if (pattern[i] == pattern[k + 1])
			k++;
		pi[i] = k;
	}
	return pi;
}

int Kmp(__in_ecount(cbTargetSize) unsigned char *lpTarget, __in size_t cbTargetSize,
	__in_ecount(cbPatternSize) unsigned char *lpPattern, __in size_t cbPatternSize)
{

	if (lpTarget == NULL || cbTargetSize == 0) return -1;
	int *pi = PrefixKmp(lpPattern, cbPatternSize);
	if (pi == NULL) return -1;

	size_t i = 0;
	int k = -1;

	for (i = 0; i < cbTargetSize; i++) {
		while (k > -1 && lpPattern[k + 1] != lpTarget[i])
			k = pi[k];
		if (lpTarget[i] == lpPattern[k + 1])
			k++;
		if ((size_t)k == cbPatternSize - 1) {
			KmpFree(pi);
			return (int)(i - (size_t)k);
		}
	}
	KmpFree(pi);
	return -1;
}