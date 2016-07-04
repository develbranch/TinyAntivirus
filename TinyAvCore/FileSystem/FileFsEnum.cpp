#include "FileFsEnum.h"
#include  <algorithm>
#include "FileFs.h"
#include "FileFsEnumContext.h"

CFileFsEnum::CFileFsEnum()
{
	m_findHandle = INVALID_HANDLE_VALUE;
	ZeroMemory(&m_wfd, sizeof(m_wfd));
	m_hStop = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CFileFsEnum::~CFileFsEnum()
{
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		m_Observers[i]->Release();
	}

	n = m_Archivers.size();
	for (i = 0; i < n; i++)
	{
		m_Archivers[i]->Release();
	}
	if (m_hStop)
	{
		CloseHandle(m_hStop);
		m_hStop = NULL;
	}
}

HRESULT WINAPI CFileFsEnum::QueryInterface(
	__in REFIID riid,
	__out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(IFsEnum)))
	{
		*ppvObject = static_cast<IFsEnum*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, __uuidof(IFsEnumObserver)))
	{
		*ppvObject = static_cast<IFsEnumObserver*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

HRESULT WINAPI CFileFsEnum::AddObserver(__in IFsEnumObserver *observer)
{
	if (observer == NULL) return E_INVALIDARG;
	if (m_Observers.end() == std::find(m_Observers.begin(), m_Observers.end(), observer))
	{
		observer->AddRef();
		m_Observers.push_back(observer);
		return S_OK;
	}

	return E_NOT_VALID_STATE;
}

HRESULT WINAPI CFileFsEnum::RemoveObserver(__in IFsEnumObserver *observer)
{
	if (observer == NULL) return E_INVALIDARG;
	std::vector<IFsEnumObserver *>::iterator it;
	it = std::find(m_Observers.begin(), m_Observers.end(), observer);
	if (m_Observers.end() == it)
	{
		return E_NOT_SET;
	}

	(*it)->Release();
	m_Observers.erase(it);
	return S_OK;
}

HRESULT WINAPI CFileFsEnum::AddArchiver(__in IFsEnum * archiver)
{
	if (archiver == NULL) return E_INVALIDARG;
	if (m_Archivers.end() == std::find(m_Archivers.begin(), m_Archivers.end(), archiver))
	{
		archiver->AddRef();
		m_Archivers.push_back(archiver);
		return S_OK;
	}

	return E_NOT_VALID_STATE;
}

HRESULT WINAPI CFileFsEnum::RemoveArchiver(__in IFsEnum * archiver)
{
	if (archiver == NULL) return E_INVALIDARG;
	std::vector<IFsEnum *>::iterator it;
	it = std::find(m_Archivers.begin(), m_Archivers.end(), archiver);
	if (m_Archivers.end() == it)
	{
		return E_NOT_SET;
	}

	(*it)->Release();
	m_Archivers.erase(it);
	return S_OK;
}

typedef struct DIRPATH
{
	StringW path;
	int depth;
}DIRPATH;

HRESULT WINAPI CFileFsEnum::Enum(__in IFsEnumContext *context)
{
	if (context == NULL) return E_INVALIDARG;

	// Use linked-list to store stack of found directories
	std::stack<DIRPATH> dirStack;

	// Search variables
	StringW	fullPath;
	DIRPATH currentDirInfo;

	HRESULT	hr = S_OK;
	bool	stopSearch = false;


	// Search context
	BSTR	searchPattern = NULL;
	BSTR	searchContainerPath = NULL;
	IVirtualFs * searchContainer = NULL;
	int		maxDepth = context->GetMaxDepth();

	if (FAILED(context->GetSearchContainer((IVirtualFs**)&searchContainer)) ||
		FAILED(context->GetSearchPattern(&searchPattern)) ||
		FAILED(searchContainer->GetFullPath(&searchContainerPath)))
	{
		if (searchContainer) searchContainer->Release();
		if (searchContainerPath) SysFreeString(searchContainerPath);
		if (searchPattern) SysFreeString(searchPattern);
		return E_FAIL;
	}

	if (wcslen(searchContainerPath) == 0 ||
		wcslen(searchPattern) == 0 ||
		(maxDepth < 0 && maxDepth != -1))
	{
		if (searchContainer) searchContainer->Release();
		if (searchContainerPath) SysFreeString(searchContainerPath);
		if (searchPattern) SysFreeString(searchPattern);
		return E_INVALIDARG;
	}

	// Initialize search stack. This stack is used to avoid recursion
	dirStack.push({ searchContainerPath, 0 });
	SysFreeString(searchContainerPath);
	searchContainerPath = NULL;

	InitArchiveObservers();
	if (EnumInit())
	{
		// Start the enumeration loop
		while (!dirStack.empty() && !stopSearch)
		{
			// Get the directory at top of the stack to start enumeration
			currentDirInfo = dirStack.top();
			dirStack.pop();

			// Check directory name and search depth
			if (currentDirInfo.path.empty()) continue;
			if (currentDirInfo.depth > maxDepth && maxDepth > 0) continue;

			//If selected object is file then will not MakeFullPathW(dir.c_str(), searchPattern)
			if (TestFilePath(currentDirInfo.path.c_str()))
			{
				hr = OnEnumEntryFound(NULL, currentDirInfo.path.c_str(), context, currentDirInfo.depth);
				if (hr == E_ABORT)
					stopSearch = true;
				
				if (FAILED(hr))
				{
					if (hr == E_NOT_SET)
						OnError(FsEnumNotFound, currentDirInfo.path.c_str());
					
					OnError(FsEnumAccessDenied, currentDirInfo.path.c_str());
				}
				continue;
			}

			// Start enumerate files and sub-directories of the current search directory
			fullPath = MakePath(currentDirInfo.path.c_str(), searchPattern);
			if (!EnumFirstFile(fullPath.c_str()))
				continue;
			IVirtualFs * entryContainer = static_cast<IVirtualFs*>(new CFileFs());
			if (entryContainer == NULL)
			{
				stopSearch = true;
				hr = E_OUTOFMEMORY;
				continue;
			}
			hr = entryContainer->Create(currentDirInfo.path.c_str(), 0);
			if (FAILED(hr))
			{
				stopSearch = true;
				continue;
			}

			do
			{
				if (!wcscmp(m_wfd.cFileName, L".") ||
					!wcscmp(m_wfd.cFileName, L".."))
					continue;	// Skip parent dir and current dir
				fullPath = MakePath(currentDirInfo.path.c_str(), m_wfd.cFileName);

				if (TEST_FLAG(m_wfd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
				{
					// Add sub-directory to search stack
					if (currentDirInfo.depth < (maxDepth - 1) || maxDepth == -1)
					{
						dirStack.push({ fullPath, currentDirInfo.depth + 1 });
					}
				}
				else
				{
					hr = OnEnumEntryFound(entryContainer, m_wfd.cFileName, context, currentDirInfo.depth + 1);
					if (hr == E_ABORT)
					{
						stopSearch = true;
						break;
					}

					if (FAILED(hr))
					{
						if (hr == E_NOT_SET)
							OnError(FsEnumNotFound, fullPath.c_str());

						OnError(FsEnumAccessDenied, fullPath.c_str());
					}
				}
				if (m_hStop && WaitForSingleObject(m_hStop, 0) == WAIT_OBJECT_0)
					stopSearch = true;
			} while (EnumNextFile() && (!stopSearch));
			EnumClose();
			entryContainer->Close();
			entryContainer->Release();
			if (m_hStop && WaitForSingleObject(m_hStop, 0) == WAIT_OBJECT_0)
				stopSearch = true;
		}
	}

	SysFreeString(searchPattern);
	CleanupArchiveObservers();
	if (searchContainer) searchContainer->Release();
	return hr;
}

// called by archivers
HRESULT WINAPI CFileFsEnum::OnFileFound(__in IVirtualFs *file, __in IFsEnumContext *context, __in const int currentDepth)
{
	size_t i, n;
	HRESULT	hr = S_OK;
	BOOL bOver = FALSE;

	if (SUCCEEDED(IsFileTooLarge(file, context, &bOver)) && bOver)
		return E_OUTOFMEMORY;

	n = m_Observers.size();
	IVirtualFs* container = NULL;
	file->GetContainer(&container);
	for (i = 0; i < n; i++)
	{
		if (m_Observers[i] == NULL) continue;

		hr = m_Observers[i]->OnFileFound(file, context, currentDepth);
		if (FAILED(hr) || (WaitForSingleObject(m_hStop, 0) == WAIT_OBJECT_0))	break;
		if (container)
		{
			hr = CheckDeferredDeletion(container, file);
			if (hr == S_FALSE) continue;
			if (SUCCEEDED(hr)) 
			{
				container->Release();
				return hr;
			}
			else break;
		}
	}
	if (container) container->Release();
	if (FAILED(hr)) return hr;

	// Enum by archivers
	EnumByArchivers(file, context, currentDepth, context->GetDepthInArchive());
	return S_OK;
}

void WINAPI CFileFsEnum::OnError(__in DWORD dwErrorCode, __in_opt LPCWSTR lpMessage /*= NULL*/)
{
	size_t i, n;
	n = m_Observers.size();
	
	for (i = 0; i < n; i++)
	{
		if (m_Observers[i] == NULL) continue;

		m_Observers[i]->OnError(dwErrorCode, lpMessage);
	}
}

HRESULT WINAPI CFileFsEnum::IsFileTooLarge(__in IVirtualFs * container, __in LPCWSTR fileName, __in IFsEnumContext *context, __out BOOL* over)
{
	HRESULT	hr = S_OK;

	if (fileName == NULL || context == NULL) return E_INVALIDARG;

	// Initialize file object
	IVirtualFs *fsFile = new CFileFs();
	if (fsFile == NULL) return E_OUTOFMEMORY;

	if (SUCCEEDED(hr = fsFile->SetContainer(container)) &&
		SUCCEEDED(hr = fsFile->Create(fileName, 0))
		)
	{
		hr = IsFileTooLarge(fsFile, context, over);
	}

	// Release the file object
	fsFile->Release();
	return hr;
}

HRESULT WINAPI CFileFsEnum::IsFileTooLarge(__in IVirtualFs * file, __in IFsEnumContext *context, __out BOOL* over)
{
	if (file == NULL || context == NULL || over == NULL) return E_INVALIDARG;
	*over = FALSE;
	HRESULT	hr = S_OK;
	ULARGE_INTEGER maxFileSize, fileSize;
	IFsAttribute * attribute = NULL;

	if (SUCCEEDED(hr = context->GetMaxFileSize(&maxFileSize)) &&
		SUCCEEDED(hr = file->QueryInterface(__uuidof(IFsAttribute), (LPVOID*)&attribute)) &&
		SUCCEEDED(hr = attribute->Size(&fileSize))
		)
	{
		*over = maxFileSize.QuadPart < fileSize.QuadPart;
	}

	if (attribute) attribute->Release();
	return hr;
}

HRESULT WINAPI CFileFsEnum::OnEnumEntryFound(__in IVirtualFs * container, __in LPCWSTR fileName, __in IFsEnumContext *context, __in int currentDepth)
{
	if (fileName == NULL || context == NULL || currentDepth < 0) return E_INVALIDARG;

	int		i, n;
	HRESULT	hr = S_OK;
	BOOL bOver = FALSE;

	if (SUCCEEDED(IsFileTooLarge(container, fileName, context, &bOver)) && bOver)
		return E_OUTOFMEMORY;

	// Initialize file object
	IVirtualFs *fsFile = new CFileFs();
	if (fsFile == NULL) return E_OUTOFMEMORY;
	ULONG creationFlags = 0;

	switch (context->GetFlags())
	{
	case IFsEnumContext::DetectOnly:
		creationFlags = IVirtualFs::fsRead | IVirtualFs::fsSharedRead | IVirtualFs::fsSharedDelete | IVirtualFs::fsOpenExisting | IVirtualFs::fsAttrNormal;
		break;

	case IFsEnumContext::Disinfect:
		creationFlags = IVirtualFs::fsRead | IVirtualFs::fsWrite | IVirtualFs::fsSharedRead | IVirtualFs::fsSharedDelete | IVirtualFs::fsOpenExisting | IVirtualFs::fsAttrNormal;
		break;

	default:
		creationFlags = 0;
		break;
	}

	if (SUCCEEDED(hr = fsFile->SetContainer(container)) &&
		SUCCEEDED(hr = fsFile->Create(fileName, creationFlags)))
	{
		// Now scan file user file scanner modules
		n = (int)m_Observers.size();
		for (i = 0; i < n; i++)
		{
			hr = m_Observers[i]->OnFileFound(fsFile, context, currentDepth);
			if (FAILED(hr) || (WaitForSingleObject(m_hStop, 0) == WAIT_OBJECT_0))	break;

			hr = CheckDeferredDeletion(container, fsFile);
			if (hr == S_FALSE) continue;
			if (SUCCEEDED(hr))
			{
				// Release the file object
				fsFile->Release();
				fsFile = NULL;
				return hr;
			}
			else break;
		}

		if ((hr != E_ABORT) && (WaitForSingleObject(m_hStop, 0) == WAIT_TIMEOUT))
		{
			// Enum by archivers
			EnumByArchivers(fsFile, context, currentDepth, 0);
			hr = CheckDeferredDeletion(container, fsFile);
			if (hr == S_OK)
			{
				// Release the file object
				fsFile->Release();
				fsFile = NULL;
			}
		}
	}

	// Release the file object
	if (fsFile) fsFile->Release();
	return hr;
}

StringW CFileFsEnum::MakePath(__in LPCWSTR str1, __in LPCWSTR str2)
{
	StringW fullPath = StringW(str1) + L"\\" + StringW(str2);
	return fullPath;
}

HRESULT CFileFsEnum::CheckDeferredDeletion(__in IVirtualFs * container, __in IVirtualFs * file)
{
	ULONG flags;
	if (SUCCEEDED(file->GetFlags(&flags)) &&
		TEST_FLAG(flags, IVirtualFs::fsDeferredDeletion))
	{
		ULONG fsType;
		file->GetFsType(&fsType);
		switch (fsType)
		{
		case IVirtualFs::archive:
			container->DeferredDelete();
			return S_OK;

		case IVirtualFs::basic:
			return S_OK;
		default:
			return E_NOT_VALID_STATE;
			break;
		}
	}
	else
	{
		return S_FALSE;
	}

}

void WINAPI CFileFsEnum::InitArchiveObservers(void)
{
	for (std::vector<IFsEnum *>::iterator it = m_Archivers.begin(); it != m_Archivers.end(); ++it)
	{
		for (std::vector<IFsEnumObserver*>::iterator observer = m_Observers.begin(); observer != m_Observers.end(); ++observer)
		{
			(*it)->AddObserver(*observer);
		}

		for (std::vector<IFsEnum*>::iterator archiver = m_Archivers.begin();
			archiver != m_Archivers.end();
			++archiver)
		{
			(*it)->AddArchiver(*archiver);
		}
	}
}

void WINAPI CFileFsEnum::EnumByArchivers(__in IVirtualFs *file, __in IFsEnumContext *context, __in const int depth, __in const int depthInArchive)
{
	if (context->GetMaxDepthInArchive() != -1 &&
		depthInArchive + 1 > context->GetMaxDepthInArchive()
		)
	{
		return;
	}

	IFsEnumContext * archiveEnum = static_cast<IFsEnumContext*>(new CFileFsEnumContext);
	if (archiveEnum == NULL) return;
		
	archiveEnum->SetSearchContainer(file);
	archiveEnum->SetFlags(context->GetFlags());
	archiveEnum->SetMaxDepth(context->GetMaxDepth());
	archiveEnum->SetDepth(depth);
	archiveEnum->SetMaxDepthInArchive(context->GetMaxDepthInArchive());
	archiveEnum->SetDepthInArchive(depthInArchive);

	BSTR s = NULL;
	if (SUCCEEDED(context->GetSearchPattern(&s)))
	{
		archiveEnum->SetSearchPattern(s);
		SysFreeString(s);
	}

	for (std::vector<IFsEnum *>::iterator it = m_Archivers.begin(); it != m_Archivers.end(); ++it)
	{
		(*it)->Enum(archiveEnum);
		if (WaitForSingleObject(m_hStop, 0) == WAIT_OBJECT_0)
			break;
		
		ULONG flags;
		if (SUCCEEDED(file->GetFlags(&flags)) &&
			TEST_FLAG(flags, IVirtualFs::fsDeferredDeletion))
		{
			break;
		}
	}

	archiveEnum->Release();
}

void WINAPI CFileFsEnum::CleanupArchiveObservers(void)
{
	for (std::vector<IFsEnum *>::iterator it = m_Archivers.begin(); it != m_Archivers.end(); ++it)
	{
		for (std::vector<IFsEnumObserver*>::iterator observer = m_Observers.begin(); observer != m_Observers.end(); ++observer)
		{
			(*it)->RemoveObserver(*observer);
		}

		for (std::vector<IFsEnum*>::iterator archiver = m_Archivers.begin();
			archiver != m_Archivers.end();
			++archiver)
		{
			(*it)->RemoveArchiver(*archiver);
		}
	}
}

BOOL WINAPI CFileFsEnum::TestFilePath(__in LPCWSTR lpFileName)
{
	BOOL isFile = FALSE;
	WIN32_FIND_DATAW wfd;
	HANDLE findHandle = FindFirstFileW(lpFileName, &wfd);
	if (findHandle != INVALID_HANDLE_VALUE)
	{
		isFile = (!TEST_FLAG(wfd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY));
		FindClose(findHandle);
	}

	return isFile;
}

BOOL WINAPI CFileFsEnum::EnumInit(void)
{
	return TRUE;
}

BOOL WINAPI CFileFsEnum::EnumFirstFile(__in LPCWSTR lpFileName)
{
	m_findHandle = FindFirstFileW(lpFileName, &m_wfd);
	return (m_findHandle != INVALID_HANDLE_VALUE);
}

BOOL WINAPI CFileFsEnum::EnumNextFile(void)
{
	if (m_findHandle == INVALID_HANDLE_VALUE) return FALSE;
	return FindNextFile(m_findHandle, &m_wfd);
}

void WINAPI CFileFsEnum::EnumClose(void)
{
	if (m_findHandle != INVALID_HANDLE_VALUE)
		FindClose(m_findHandle);
	m_findHandle = INVALID_HANDLE_VALUE;
}

void WINAPI CFileFsEnum::Stop(void)
{
	if (m_hStop)
		SetEvent(m_hStop);
}
