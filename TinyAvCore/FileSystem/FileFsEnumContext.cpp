#include "FileFsEnumContext.h"

CFileFsEnumContext::CFileFsEnumContext()
{
	m_depth = 0;
	m_maxDepth = -1;
	m_ArchiveDepth = 0;
	m_maxArchiveDepth = -1;
	m_flags = 0;
	m_container = NULL;
	m_maxSize.QuadPart = MAX_FILE_SIZE;
}

CFileFsEnumContext::~CFileFsEnumContext()
{
	if (m_container)
	{
		m_container->Release();
		m_container = NULL;
	}
}

HRESULT WINAPI CFileFsEnumContext::QueryInterface(
	__in REFIID riid, 
	__out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(IFsEnumContext)))
	{
		*ppvObject = static_cast<IFsEnumContext*>(this);
		AddRef();
		return S_OK;
	}
	else
	{
		*ppvObject = NULL;
	}
	return E_NOINTERFACE;
}

HRESULT WINAPI CFileFsEnumContext::SetSearchContainer(__in IVirtualFs *container)
{
	if (container == NULL) return E_INVALIDARG;

	if (m_container)
	{
		m_container->Release();
		m_container = NULL;
	}

	container->AddRef();
	m_container = container;
	return S_OK;
}

HRESULT WINAPI CFileFsEnumContext::GetSearchContainer(__out IVirtualFs **container)
{
	if (container == NULL) return E_INVALIDARG;
	*container = m_container;
	m_container->AddRef();
	return S_OK;
}

HRESULT WINAPI CFileFsEnumContext::SetSearchPattern(__in LPCWSTR searchPattern)
{
	if (searchPattern == NULL) return E_INVALIDARG;
	m_searchPattern = searchPattern;
	return S_OK;
}

HRESULT WINAPI CFileFsEnumContext::GetSearchPattern(__out BSTR *searchPattern)
{
	if (m_searchPattern.length() == 0) return E_NOT_SET;
	if (searchPattern == NULL) return E_INVALIDARG;

	*searchPattern = SysAllocString(m_searchPattern.c_str());
	return (*searchPattern) ? S_OK : E_OUTOFMEMORY;
}

int WINAPI CFileFsEnumContext::GetMaxDepth(void)
{
	return m_maxDepth;
}

int WINAPI CFileFsEnumContext::GetDepth(void)
{
	return m_depth;
}

HRESULT WINAPI CFileFsEnumContext::SetMaxFileSize(__in ULARGE_INTEGER fileSize)
{
	if (fileSize.QuadPart <= MAX_FILE_SIZE)
	{
		m_maxSize = fileSize;
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

HRESULT WINAPI CFileFsEnumContext::GetMaxFileSize(__in ULARGE_INTEGER *fileSize)
{
	if (fileSize == NULL) return E_INVALIDARG;
	*fileSize = m_maxSize;
	return S_OK;
}

HRESULT WINAPI CFileFsEnumContext::AddIgnoreItem(__in LPCWSTR lpPath)
{
	if (lpPath == NULL) return E_INVALIDARG;
	if (m_ignore.end() == std::find(m_ignore.begin(), m_ignore.end(), StringW(lpPath)))
	{
		m_ignore.push_back(lpPath);
		return S_OK;
	}
	return S_FALSE;
}

HRESULT WINAPI CFileFsEnumContext::RemoveIgnoreItem(__in LPCWSTR lpPath)
{
	if (lpPath == NULL) return E_INVALIDARG;
	std::vector<StringW>::iterator it = std::find(m_ignore.begin(), m_ignore.end(), StringW(lpPath));
	if (m_ignore.end() == it)
	{
		return E_NOT_SET;
	}
	m_ignore.erase(it);
	return S_FALSE;
}

HRESULT WINAPI CFileFsEnumContext::GetIgnoreList(__out BSTR* lpPath, __out UINT *itemCount)
{
	if (itemCount == NULL || lpPath == NULL) return E_INVALIDARG;
	*itemCount = (UINT)m_ignore.size();
	if (*itemCount == 0) return S_OK;
	*lpPath = (BSTR)new BSTR[*itemCount];
	if (*lpPath == NULL)
		return E_OUTOFMEMORY;
	ZeroMemory(*lpPath, *itemCount * sizeof(BSTR));
	for (UINT i = 0; i < *itemCount; ++i)
	{
		BSTR b = SysAllocString(m_ignore[i].c_str());
		if (b == NULL)
		{
			for (UINT j = 0; j < *itemCount; ++j)
			{
				if (lpPath[j])
					SysFreeString(lpPath[j]);
			}
			delete[] (*lpPath);
			return E_OUTOFMEMORY;
		}

		lpPath[i] = b;
	}
	
	return S_OK;
}

HRESULT WINAPI CFileFsEnumContext::FreeIgnoreList(__in BSTR* lpPath, __in UINT itemCount)
{
	for (UINT j = 0; j < itemCount; ++j)
	{
		if (lpPath[j])
			SysFreeString(lpPath[j]);
	}
	delete[](*lpPath);
	return S_OK;
}

int WINAPI CFileFsEnumContext::GetMaxDepthInArchive(void)
{
	return m_maxArchiveDepth;
}

int WINAPI CFileFsEnumContext::GetDepthInArchive(void)
{
	return m_ArchiveDepth;
}

HRESULT WINAPI CFileFsEnumContext::SetMaxDepthInArchive(__in const int maxDepth)
{
	m_maxArchiveDepth = maxDepth;
	return S_OK;
}

HRESULT WINAPI CFileFsEnumContext::SetDepthInArchive(__in const int depth)
{
	m_ArchiveDepth = depth;
	return S_OK;
}

HRESULT WINAPI CFileFsEnumContext::SetMaxDepth(__in const int maxDepth)
{
	m_maxDepth = maxDepth;
	return S_OK;
}

HRESULT WINAPI CFileFsEnumContext::SetDepth(__in const int depth)
{
	m_depth = depth;
	return S_OK;
}

HRESULT WINAPI CFileFsEnumContext::SetFlags(__in const ULONG flags)
{
	m_flags = flags;
	return S_OK;
}

ULONG WINAPI CFileFsEnumContext::GetFlags(void)
{
	return m_flags;
}

