#pragma once
#include <tchar.h>
#include <Windows.h>

class CRefCount
{
private:
	mutable LONG m_RefCount;

public:

	CRefCount(void) : m_RefCount(1)
	{
	}

	virtual ~CRefCount(void) { }

	virtual ULONG WINAPI AddRef(void)
	{
		return InterlockedIncrement(&m_RefCount);
	}

	virtual ULONG WINAPI Release(void)
	{
		if (!InterlockedDecrement(&m_RefCount))
		{
			delete this;
			return 0;
		}
		return m_RefCount;
	}

	virtual ULONG WINAPI  GetCount(void)
	{
		return m_RefCount;
	}
};


#if !defined DECLARE_REF_COUNT

#define DECLARE_REF_COUNT() \
	public: \
	virtual ULONG WINAPI GetCount( void ) { return CRefCount::GetCount(); } \
	virtual ULONG WINAPI AddRef( void )  { return CRefCount::AddRef(); } \
	virtual ULONG WINAPI Release( void )  { return CRefCount::Release(); } \

#endif