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

	// notify observer before scanning file
	hr = observer->OnPreScan(file, context);
	if (FAILED(hr)) return hr; // failed --> return

	// check for PE file type
	hr = m_parser->CheckType(file, &isMatched);
	if (FAILED(hr) || isMatched == FALSE) return hr; // not PE file or malformed 

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

	BYTE * sality = new BYTE[0x100];
	if (sality == NULL) return;

	// check sality code
	if (FAILED(m_emul->ReadMemory((DWORD_PTR)salityEp, sality, 0x100)) ||
		!VerifySignature(sality, 0x100)
		)
	{
		delete[] sality;
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
	delete[] sality;
}

BOOL CKillVirus::VerifySignature(__in_bcount(size) LPBYTE buffer, __in DWORD const size)
{
	static unsigned char signature1[] = {
		0xE8, 0x00, 0x00, 0x00, 0x00, 0x5D, 0x8B, 0xC5,
		0x81, 0xED, 0x05, 0x10, 0x40, 0x00, 0x8A, 0x9D,
		0x73, 0x27, 0x40, 0x00, 0x84, 0xDB, 0x74, 0x13,
		0x81, 0xC4 };
	if (memcmp(buffer, signature1, sizeof(signature1))) return FALSE;

	static unsigned char signature2[] = {
		0x89, 0x85, 0x54, 0x12, 0x40, 0x00, 0xEB, 0x19,
		0xC7, 0x85, 0x4D, 0x14, 0x40, 0x00, 0x22, 0x22,
		0x22, 0x22, 0xC7, 0x85, 0x3A, 0x14, 0x40, 0x00,
		0x33, 0x33, 0x33, 0x33, 0xE9, 0x82, 0x00, 0x00,
		0x00, 0x33, 0xDB, 0x64, 0x67, 0x8B, 0x1E, 0x30,
		0x00, 0x85, 0xDB, 0x78, 0x0E, 0x8B, 0x5B, 0x0C };
	if (memcmp(buffer + 0x23, signature2, sizeof(signature2))) return FALSE;

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
