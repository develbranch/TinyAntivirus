#include "PeEmulator.h"

CPeEmulator::CPeEmulator()
{
	m_engine = NULL;
	m_bEmulatorEngineReady = uc_dyn_load(NULL, 0);
	m_starting = false;
}

CPeEmulator::~CPeEmulator()
{
	size_t i, n;
	n = m_Observers.size();
	for (i = 0; i < n; i++)
	{
		m_Observers[i]->Release();
	}

	uc_dyn_free();
}

HRESULT WINAPI CPeEmulator::OnStarting(void)
{
	HRESULT hr;
	std::vector<IEmulObserver *>::iterator it;
	for (it = m_Observers.begin(); it != m_Observers.end(); ++it)
	{
		hr = (*it)->OnEmulatorStarting();
		if (FAILED(hr)) return hr;
	}

	m_starting = true;
	return S_OK;
}

void WINAPI CPeEmulator::OnError(__in DWORD const dwErrorCode)
{
	std::vector<IEmulObserver *>::iterator it;
	for (it = m_Observers.begin(); it != m_Observers.end(); ++it)
	{
		(*it)->OnError(dwErrorCode);
	}
}

HRESULT WINAPI CPeEmulator::OnStopped(void)
{
	if (m_starting)
	{
		HRESULT hr;
		std::vector<IEmulObserver *>::iterator it;
		for (it = m_Observers.begin(); it != m_Observers.end(); ++it)
		{
			hr = (*it)->OnEmulatorStopped();
			if (FAILED(hr)) return hr;
		}
		m_starting = false;
	}
	return S_OK;
}

HRESULT WINAPI CPeEmulator::QueryInterface(__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(IEmulator)))
	{
		*ppvObject = static_cast<IEmulator*>(this);
		this->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT WINAPI CPeEmulator::ReadRegister(__in DWORD const reg, __out DWORD_PTR *regValue)
{
	if (m_engine == NULL) return E_NOT_VALID_STATE;
	return (uc_reg_read(m_engine, (int)reg, (void*)regValue) == UC_ERR_OK) ? S_OK : E_FAIL;
}

HRESULT WINAPI CPeEmulator::WriteRegister(__in DWORD const reg, __out DWORD_PTR const regValue)
{
	if (m_engine == NULL) return E_NOT_VALID_STATE;
	return (uc_reg_write(m_engine, (int)reg, (void*)&regValue) == UC_ERR_OK) ? S_OK : E_FAIL;
}

HRESULT WINAPI CPeEmulator::ReadMemory(__in DWORD_PTR memoryAddr, __out_bcount(nNumberOfBytesToRead) LPVOID lpBuffer, __in DWORD nNumberOfBytesToRead)
{
	if (m_engine == NULL) return E_NOT_VALID_STATE;
	uc_err err = uc_mem_read(m_engine, (uint64_t)memoryAddr, (void*)lpBuffer, (size_t)nNumberOfBytesToRead);
	return (err == UC_ERR_OK) ? S_OK : E_FAIL;
}

HRESULT WINAPI CPeEmulator::WriteMemory(__in DWORD_PTR memoryAddr, __out_bcount(nNumberOfBytesToWrite) LPVOID lpBuffer, __in DWORD nNumberOfBytesToWrite)
{
	if (m_engine == NULL) return E_NOT_VALID_STATE;
	uc_err err = uc_mem_write(m_engine, (uint64_t)memoryAddr, (void*)lpBuffer, (size_t)nNumberOfBytesToWrite);
	return (err == UC_ERR_OK) ? S_OK : E_FAIL;
}

HRESULT WINAPI CPeEmulator::EmulateCode(
	__in_bcount(nSizeOfCode) LPBYTE lpCodeBuffer, __in DWORD nSizeOfCode,
	__in DWORD_PTR memoryMappedAddr, __in DWORD nSizeOfStackCommit, __in DWORD nSizeOfStackReserve,
	__in DWORD_PTR addressToStart, __in DWORD nNumberOfBytesToEmulate)
{
	uc_err err;
	HRESULT hr;

	if (m_bEmulatorEngineReady == false)
	{
		OnError(IEmulObserver::EmulatorIsNotFound);
		return E_NOT_VALID_STATE;
	}

	try
	{
		err = uc_open(UC_ARCH_X86, UC_MODE_32, &m_engine);
		if (err != UC_ERR_OK) {
			OnError(IEmulObserver::EmulatorIsNotRunable);
			return E_FAIL;
		}

		if (FAILED(hr = OnStarting()))
		{
			uc_close(m_engine);
			m_engine = NULL;
			return hr;
		}

		// map memory for this emulation
		err = uc_mem_map(m_engine, memoryMappedAddr, nSizeOfCode, UC_PROT_ALL);
		if (err != UC_ERR_OK)
		{
			OnStopped();
			uc_close(m_engine);
			m_engine = NULL;
			return E_FAIL;
		}

		err = uc_mem_map(m_engine, memoryMappedAddr - nSizeOfStackReserve, nSizeOfStackCommit, UC_PROT_READ | UC_PROT_WRITE);
		if (err != UC_ERR_OK)
		{
			OnStopped();
			uc_close(m_engine);
			m_engine = NULL;
			return E_FAIL;
		}

		DWORD r_esp = (DWORD)memoryMappedAddr - nSizeOfStackReserve + nSizeOfStackCommit;
		err = uc_reg_write(m_engine, UC_X86_REG_ESP, &r_esp);
		if (err != UC_ERR_OK)
		{
			OnStopped();
			uc_close(m_engine);
			m_engine = NULL;
			return E_FAIL;
		}

		err = uc_mem_write(m_engine, memoryMappedAddr, lpCodeBuffer, nSizeOfCode);
		if (err != UC_ERR_OK)
		{
			OnStopped();
			uc_close(m_engine);
			m_engine = NULL;
			return E_FAIL;
		}

		// emulate machine code in infinite time
		if (nNumberOfBytesToEmulate == 0)
			err = uc_emu_start(m_engine, addressToStart, 0, 0, 0);
		else
			err = uc_emu_start(m_engine, addressToStart, addressToStart + nNumberOfBytesToEmulate - 1, 0, 0);
	
		OnStopped();
		uc_close(m_engine);
		m_engine = NULL;

		return (err == UC_ERR_OK) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		if (m_starting)
		{
			OnError(IEmulObserver::EmulatorInternalError);
			OnStopped();
			if (m_engine)
			{
				uc_close(m_engine);
				m_engine = NULL;
			}
		}
		else
		{
			OnError(IEmulObserver::EmulatorInternalError);
			m_engine = NULL;
		}

		return E_FAIL;
	}
}

HRESULT WINAPI CPeEmulator::EmulatePeFile(__in IPeFile *peFile, __in DWORD_PTR rvaToStart, __in int origin, __in DWORD nNumberOfBytesToEmulate /*= 0*/)
{
	IMAGE_SECTION_HEADER section;
	IMAGE_NT_HEADERS32 ntHeader;
	IFsStream * fileStream = NULL;
	ULONG readSize = 0;
	ULARGE_INTEGER pos;
	LARGE_INTEGER distanceToMove = {};

	HRESULT hr = peFile->GetPEHeader(&ntHeader);
	if (FAILED(hr)) return hr;

	BYTE *memPe = new BYTE[ntHeader.OptionalHeader.SizeOfImage];
	if (memPe == NULL)
	{
		return E_OUTOFMEMORY;
	}

	ZeroMemory(memPe, ntHeader.OptionalHeader.SizeOfImage);

	hr = peFile->QueryInterface(__uuidof(IFsStream), (LPVOID*)&fileStream);
	if (FAILED(hr))
	{
		delete[] memPe;
		return hr;
	}

	hr = fileStream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin);
	if (FAILED(hr))
	{
		fileStream->Release();
		delete[] memPe;
		return hr;
	}

	hr = peFile->GetSectionHeader(0, &section);
	if (FAILED(hr))
	{
		fileStream->Release();
		delete[] memPe;
		return hr;
	}

	hr = fileStream->Read(memPe, section.PointerToRawData, &readSize);
	if (FAILED(hr) || readSize != section.PointerToRawData)
	{
		fileStream->Release();
		delete[] memPe;
		if (FAILED(hr)) return hr;
		return E_FAIL;
	}

	for (UINT i = 0; i < peFile->GetSectionCount(); ++i)
	{
		hr = peFile->GetSectionHeader(i, &section);
		if (FAILED(hr))
		{
			fileStream->Release();
			delete[] memPe;
			return hr;
		}

		if (section.SizeOfRawData == 0)
			continue;

		distanceToMove.QuadPart = (LONGLONG)section.PointerToRawData;
		hr = fileStream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin);
		if (FAILED(hr))
		{
			fileStream->Release();
			delete[] memPe;
			return hr;
		}

		hr = fileStream->Read(memPe + section.VirtualAddress, section.SizeOfRawData, &readSize);
		if (FAILED(hr) || readSize != section.SizeOfRawData)
		{
			fileStream->Release();
			delete[] memPe;
			if (FAILED(hr)) return hr;
			return E_FAIL;
		}
	}

	uint64_t begin = 0;
	switch (origin)
	{
	case  FromEntryPoint:
		begin = ntHeader.OptionalHeader.ImageBase + ntHeader.OptionalHeader.AddressOfEntryPoint + rvaToStart;
		break;
	case FromImageBase:
		begin = ntHeader.OptionalHeader.ImageBase + rvaToStart;
		break;
	default:
		begin = ntHeader.OptionalHeader.ImageBase + ntHeader.OptionalHeader.AddressOfEntryPoint;
	}

	if (nNumberOfBytesToEmulate == 0)
	{
		IMAGE_SECTION_HEADER sectionHeader;
		UINT sectionIndex;
		if (SUCCEEDED(peFile->FindSectionByVa((UINT)begin, &sectionIndex)))
		{
			if (SUCCEEDED(peFile->GetSectionHeader(sectionIndex, &sectionHeader)))
			{
				nNumberOfBytesToEmulate = ntHeader.OptionalHeader.ImageBase + sectionHeader.VirtualAddress + sectionHeader.Misc.VirtualSize - (UINT)begin;
			}
		}
	}

	hr = EmulateCode(memPe, ntHeader.OptionalHeader.SizeOfImage,
		(DWORD_PTR)ntHeader.OptionalHeader.ImageBase,
		ntHeader.OptionalHeader.SizeOfStackCommit, ntHeader.OptionalHeader.SizeOfStackReserve,
		(DWORD_PTR)begin, nNumberOfBytesToEmulate);

	delete[] memPe;
	memPe = NULL;
	fileStream->Release();
	return hr;
}

HRESULT WINAPI CPeEmulator::AddObserver(__in IEmulObserver *observer)
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

HRESULT WINAPI CPeEmulator::RemoveObserver(__in IEmulObserver *observer)
{
	if (observer == NULL) return E_INVALIDARG;
	std::vector<IEmulObserver *>::iterator it;
	it = std::find(m_Observers.begin(), m_Observers.end(), observer);
	if (m_Observers.end() == it)
	{
		return E_NOT_SET;
	}

	(*it)->Release();
	m_Observers.erase(it);
	return S_OK;
}

HRESULT __cdecl CPeEmulator::AddHook(__out void *hookHandler, __in int type, __in void * callback, __in void *user_data, ...)
{
	if (m_engine == NULL) return E_NOT_VALID_STATE;

	va_list valist;
	uc_err ret = UC_ERR_OK;
	int id;
	uint64_t begin, end;
	va_start(valist, user_data);

	switch (type) {
		// note this default case will capture any combinations of
		// UC_HOOK_MEM_*_PROT and UC_HOOK_MEM_*_UNMAPPED
		// as well as any combination of
		// UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE and UC_HOOK_MEM_FETCH
	default:
	case UC_HOOK_INTR:
		// all combinations of UC_HOOK_MEM_*_PROT and UC_HOOK_MEM_*_UNMAPPED are caught by 'default'
	case UC_HOOK_MEM_READ_UNMAPPED:
	case UC_HOOK_MEM_WRITE_UNMAPPED:
	case UC_HOOK_MEM_FETCH_UNMAPPED:
	case UC_HOOK_MEM_READ_PROT:
	case UC_HOOK_MEM_WRITE_PROT:
	case UC_HOOK_MEM_FETCH_PROT:
		// 0 extra args
		ret = uc_hook_add(m_engine, (uc_hook *)hookHandler, type, callback, user_data);
		break;
	case UC_HOOK_INSN:
		// 1 extra arg
		id = va_arg(valist, int);
		ret = uc_hook_add(m_engine, (uc_hook *)hookHandler, type, callback, user_data, id);
		break;

	case UC_HOOK_CODE:
	case UC_HOOK_BLOCK:
	case UC_HOOK_MEM_READ:
	case UC_HOOK_MEM_WRITE:
	case UC_HOOK_MEM_FETCH:
	case UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE:
		begin = va_arg(valist, uint64_t);
		end = va_arg(valist, uint64_t);
		ret = uc_hook_add(m_engine, (uc_hook *)hookHandler, type, callback, user_data, begin, end);
		break;
	}

	va_end(valist);
	return (ret == UC_ERR_OK) ? S_OK : E_FAIL;
}

HRESULT WINAPI CPeEmulator::RemoveHook(__in size_t hookHandler)
{
	if (m_engine == NULL) return E_NOT_VALID_STATE;
	return (UC_ERR_OK == uc_hook_del(m_engine, (uc_hook)hookHandler)) ? S_OK : E_FAIL;
}

HRESULT WINAPI CPeEmulator::StopEmulator(void)
{
	if (m_engine == NULL) return E_NOT_VALID_STATE;
	return (UC_ERR_OK == uc_emu_stop(m_engine)) ? S_OK : E_FAIL;
}