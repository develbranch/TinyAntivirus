#include "KillVirus.h"

extern HMODULE g_hMod;
const ULONGLONG g_maxInsCount = 1000 * 1000 * 1000;

CKillVirus::CKillVirus()
{
	m_info.handle = g_hMod;
	m_info.type = ScanModule;
	wcscpy_s(m_info.name, MAX_NAME, L"W32.Sality.PE");
	m_parser = NULL;
	m_emul = NULL;
	ZeroMemory(&m_scanResult, sizeof(m_scanResult));
	m_InsCount = 0;
	m_OepCode = NULL;
	m_dwOepCodeSize = 0;
	m_OepAddr = 0;
	m_emulErrCode = 0;
}

CKillVirus::~CKillVirus()
{
	if (m_emul)
	{
		m_emul->Release();
		m_emul = NULL;
	}

	if (m_parser)
	{
		m_parser->Release();
		m_parser = NULL;
	}
	if (m_OepCode)
	{
		delete[] m_OepCode;
		m_OepCode = NULL;
	}
}

HRESULT WINAPI CKillVirus::QueryInterface(__in REFIID riid, __out void **ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(IScanModule)))
	{
		*ppvObject = static_cast<IScanModule*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, __uuidof(IEmulObserver)))
	{
		*ppvObject = static_cast<IEmulObserver*>(this);
		this->AddRef();
		return S_OK;
	}
	else
	{
		*ppvObject = NULL;
	}
	return E_NOINTERFACE;
}

HRESULT WINAPI CKillVirus::GetModuleInfo(__out MODULE_INFO * scanInfo)
{
	if (scanInfo == NULL) return E_INVALIDARG;
	*scanInfo = m_info;
	return S_OK;
}

ModuleType WINAPI CKillVirus::GetType(void)
{
	return m_info.type;
}

HRESULT WINAPI CKillVirus::GetName(__out BSTR *name)
{
	if (name == NULL) return E_INVALIDARG;
	*name = SysAllocString(m_info.name);
	return (*name == NULL) ? E_OUTOFMEMORY : S_OK;
}

HRESULT WINAPI CKillVirus::OnScanInitialize(void)
{
	HRESULT hr = CreateClassObject(CLSID_CPeFileParser, 0, __uuidof(IPeFile), (LPVOID*)&m_parser);
	if (FAILED(hr)) return hr;

	hr = CreateClassObject(CLSID_CPeEmulator, 0, __uuidof(IEmulator), (LPVOID*)&m_emul);
	if (FAILED(hr))
	{
		m_parser->Release();
		m_parser = NULL;
		return hr;
	}

	hr = m_emul->AddObserver(static_cast<IEmulObserver*>(this));
	if (FAILED(hr))
	{
		m_parser->Release();
		m_parser = NULL;
		m_emul->Release();
		m_emul = NULL;
		return hr;
	}
	return S_OK;
}

HRESULT WINAPI CKillVirus::Scan(__in IVirtualFs * file, __in IFsEnumContext * context, __in IScanObserver * observer)
{
	BOOL isMatched = FALSE;
	HRESULT hr;
	if (m_OepCode)
	{
		delete[] m_OepCode;
		m_OepCode = NULL;
	}
	m_dwOepCodeSize = 0;
	m_OepAddr = 0;
	m_salityEp = 0;
	m_InsCount = 0;
	m_scanResult.scanResult = NoVirus;
	m_scanResult.cleanResult = DonotClean;

	// notify observer before scanning a file
	hr = observer->OnPreScan(file, context);
	if (FAILED(hr)) return hr; // failed --> return

	// check for PE file type
	hr = m_parser->CheckType(file, &isMatched);
	if (FAILED(hr) || isMatched == FALSE) return hr; // not PE file or malformed 

	// check header of file
	IMAGE_NT_HEADERS32 peHeader;
	hr = m_parser->GetPEHeader(&peHeader);
	if (FAILED(hr)) goto Exit;
	// do not scan dll files
	if (TEST_FLAG(peHeader.OptionalHeader.DllCharacteristics, IMAGE_FILE_DLL))
		goto Exit;

	// do not scan driver images
// 	if (peHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress && peHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
// 	{
//		If it imports from "ntoskrnl.exe" or other kernel components it should be a driver
// 	}

	m_emulErrCode = 0;
	// emulate code from entry point to end of section
	hr = m_emul->EmulatePeFile(m_parser, 0, IEmulator::FromEntryPoint, 0);

	// emulator reports error
	if (m_emulErrCode) observer->OnError(m_emulErrCode);

	if (FAILED(hr) ||
		(m_scanResult.scanResult != VirusDetected))
	{
		goto Exit;
	}

	wcscpy_s(m_scanResult.malwareName, MAX_NAME, m_info.name); // get virus name

	// check scan context
	if (TEST_FLAG(context->GetFlags(), IFsEnumContext::Disinfect) == FALSE) // Scan virus only
	{
		m_scanResult.cleanResult = CleanVirusDenied;
		hr = S_OK;
		goto Exit;
	}

	hr = observer->OnPreClean(file, context, &m_scanResult);   // notify observer before cleaning file
	if (FAILED(hr)) goto Exit;  // leave it alone!

	ULONG fsType;
	hr = file->GetFsType(&fsType);
	if (FAILED(hr)) // can not get file type
	{
		m_scanResult.cleanResult = CleanVirusDenied;
		hr = E_FAIL; // failed
		goto Exit;
	}

	/* virus has been detected inside an archive
	   I will improve Archiver module to disinfect virus. Now, I can only delete it.
	*/
	if (fsType == IVirtualFs::archive)
	{
		file->DeferredDelete();
		m_scanResult.cleanResult = VirusDeleted;
		hr = S_OK;
		goto Exit;
	}
	else
	{
		if (m_dwOepCodeSize == 1) // Patient Zero --> delete it
		{
			file->DeferredDelete(); // just delete it
			m_scanResult.cleanResult = VirusDeleted;
			hr = S_OK;
			goto Exit;
		}

		IFsStream * fileStream = NULL;
		if (FAILED(file->QueryInterface(__uuidof(IFsStream), (LPVOID*)&fileStream)))
		{
			m_scanResult.cleanResult = CleanVirusDenied;
			hr = E_FAIL; // failed
			goto Exit;
		}

		hr = E_FAIL;
		if (m_OepAddr && m_OepCode)// found Original EntryPoint
		{
			UINT fileOffset = 0;
			if (SUCCEEDED(hr = m_parser->VaToFileOffset(m_OepAddr, &fileOffset)))
			{
				LARGE_INTEGER epOffset = {};
				epOffset.QuadPart = fileOffset;  // restore original code
				if (SUCCEEDED(hr = fileStream->WriteAt(epOffset, IFsStream::FsStreamBegin, m_OepCode, m_dwOepCodeSize, NULL)) &&
					SUCCEEDED(hr = m_parser->SetVaToEntryPoint(m_OepAddr)))
				{
					hr = S_FALSE; // re-scan
					m_scanResult.cleanResult = CleanVirusSucceeded;
					if (m_salityEp)
						m_parser->Truncate(m_salityEp - 0x1116); // remove virus code
				}
				else
				{
					m_scanResult.cleanResult = CleanVirusDenied;
				}
			}
		}
		fileStream->Release();
		goto Exit;
	}

Exit:
	observer->OnPostClean(file, context, &m_scanResult);
	m_parser->ReleaseCurrentFile();
	return hr;
}

HRESULT WINAPI CKillVirus::OnScanShutdown(void)
{
	if (m_emul)
	{
		m_emul->RemoveObserver(static_cast<IEmulObserver*>(this));
		m_emul->Release();
		m_emul = NULL;
	}

	if (m_parser)
	{
		m_parser->Release();
		m_parser = NULL;
	}
	return S_OK;
}

HRESULT WINAPI CKillVirus::OnEmulatorStarting(void)
{
	if (m_emul == NULL || m_parser == NULL) return E_NOT_VALID_STATE;

	HRESULT hr = m_emul->AddHook(&m_hookcode, UC_HOOK_CODE, &CKillVirus::HookCode, this, 1, 0);
	if (FAILED(hr)) return hr;
	hr = m_emul->AddHook(&m_hookmem, UC_HOOK_MEM_READ_UNMAPPED | UC_HOOK_MEM_WRITE_UNMAPPED, &CKillVirus::HookMemInvalid, this);
	if (FAILED(hr)) return hr;
	return S_OK;
}

HRESULT WINAPI CKillVirus::OnEmulatorStopped(void)
{
	if (m_hookmem)
	{
		m_emul->RemoveHook(m_hookmem);
		m_hookmem = 0;
	}

	if (m_hookcode)
	{
		m_emul->RemoveHook(m_hookcode);
		m_hookcode = 0;
	}

	return S_OK;
}

void CKillVirus::OnHookCode(uint64_t address, uint32_t size)
{
	BOOL detected = FALSE;
	m_InsCount++;

	if (m_InsCount > g_maxInsCount)
	{
		m_emul->StopEmulator();
		return;
	}

	// check for RETN instruction
	unsigned char opCode;

	// check instruction size
	if (size != 1) return;
	// check instruction code
	if (FAILED(m_emul->ReadMemory((DWORD_PTR)address, &opCode, size))) return;
	if (opCode != 0xc3) return;

	// find VA of new state
	uint32_t salityEp = 0;
	if (FAILED(m_emul->ReadRegister(UC_X86_REG_ESP, (DWORD_PTR *)&salityEp)) ||
		FAILED(m_emul->ReadMemory((DWORD_PTR)salityEp, &salityEp, sizeof(salityEp)))
		)
		return;

	// check sality code
	if ( !VerifySignature(salityEp) )
	{
		return;
	}

	// virus FOUND!
	m_scanResult.scanResult = VirusDetected;

	// find original code
	if (SUCCEEDED(m_emul->ReadMemory((DWORD_PTR)salityEp + 0x1F, &m_OepAddr, sizeof(DWORD))))
	{
		m_OepAddr = salityEp + 5 - m_OepAddr;
		unsigned char bRestoreOepCode = 0;

		if (SUCCEEDED(m_emul->ReadMemory((DWORD_PTR)salityEp + 0x1773, &bRestoreOepCode, sizeof(bRestoreOepCode))) &&
			bRestoreOepCode &&
			SUCCEEDED(m_emul->ReadMemory((DWORD_PTR)salityEp + 0x1774, &m_dwOepCodeSize, sizeof(m_dwOepCodeSize))))
		{
			if (m_dwOepCodeSize > 1)
			{
				// original code Found!
				m_OepCode = new BYTE[m_dwOepCodeSize];
				if (m_OepCode)
				{
					m_salityEp = salityEp;
					if (FAILED(m_emul->ReadMemory((DWORD_PTR)salityEp + 0x1778, m_OepCode, m_dwOepCodeSize)))
					{
						delete[] m_OepCode;
						m_OepCode = NULL;
						m_dwOepCodeSize = 0;
					}
				}
			}
		}
	}
	m_emul->StopEmulator();
}

BOOL CKillVirus::VerifySignature(__in uint32_t salityEp)
{
	int i = 0;
	
	static unsigned char signature0[] = {
		0xE8, 0x00, 0x00, 0x00, 0x00, 0x5D, 
	};
	unsigned char sality1[sizeof(signature0)];


	if (FAILED(m_emul->ReadMemory((DWORD_PTR)salityEp, sality1, sizeof(signature0))))
		return FALSE;

	if (memcmp(sality1, signature0, sizeof(signature0))) return FALSE;

	size_t codeSize = 0x200;
	unsigned char * sality = new unsigned char[codeSize];
	if (FAILED(m_emul->ReadMemory((DWORD_PTR)salityEp, sality, (DWORD)codeSize)))
	{
		delete[] sality;
		return FALSE;
	}

	static unsigned char signature1[] = {
		0x81, 0xED, 0x05, 0x10, 0x40, 0x00,
	};

	i = Kmp(sality, codeSize, signature1, sizeof(signature1));
	if (i == -1 || i > 0x20)
	{
		delete[] sality;
		return FALSE;
	}


	static unsigned char signature2[] = {
		0x8B, 0x5B, 0x0C, 0x8B, 0x5B, 0x1C, 0x8B, 0x1B,
		0x8B, 0x5B, 0x08, 0xF8, 0xEB, 0x0A, 0x8B, 0x5B,
		0x34, 0x8D, 0x5B, 0x7C, 0x8B, 0x5B, 0x3C, 0xF8,
		0x66, 0x81, 0x3B, 0x4D, 0x5A
	};
	
	 
	i = Kmp(sality, codeSize, signature2, sizeof(signature2));
	if (i == -1 || i > 0x100) 
	{
		delete[] sality;
		return FALSE;
	}


	static unsigned char signature3[] = {
		0x57, 0x68, 0x00, 0x80, 0x00, 0x00, 0x33, 0xC0, 0x50, 0x6A, 0x04, 0x50, 0x48, 0x50, 0xff, 0x95
	};

	i = Kmp(sality, codeSize, signature3, sizeof(signature3));
	if (i == -1 || i > 0x150)
	{
		delete[] sality;
		return FALSE;
	}

	unsigned char signature4[14] = {
		0x68, 0x00, 0x54, 0x01, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x06, 0x50, 0xFF, 0x95
	};


	i = Kmp(sality, codeSize, signature4, sizeof(signature4));
	if (i == -1 || i > 0x200 || i < 0x100)
	{
		delete[] sality;
		return FALSE;
	}
	delete[] sality;
	return TRUE;
}

void CKillVirus::HookCode(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{
	UNREFERENCED_PARAMETER(uc);
	CKillVirus * t = (CKillVirus*)(user_data);
	t->OnHookCode(address, size);
}

// callback for tracing memory access (READ or WRITE)
bool CKillVirus::HookMemInvalid(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data)
{
	UNREFERENCED_PARAMETER(uc);
	UNREFERENCED_PARAMETER(type);
	UNREFERENCED_PARAMETER(address);
	UNREFERENCED_PARAMETER(size);
	UNREFERENCED_PARAMETER(value);
	return false;
}

void WINAPI CKillVirus::OnError(__in DWORD const dwErrorCode)
{
	m_emulErrCode = dwErrorCode;
}
