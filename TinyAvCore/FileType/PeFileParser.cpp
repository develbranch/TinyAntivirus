#include "PeFileParser.h"

CPeFileParser::CPeFileParser()
{
	ZeroMemory(&m_dosHeader, sizeof(m_dosHeader));
	ZeroMemory(&m_peHeader, sizeof(m_peHeader));
	m_SectionCount = 0;
	m_SectionTable = NULL;
	m_typeMatched = FALSE;
	m_stream = NULL;
	m_file = NULL;
}

CPeFileParser::~CPeFileParser()
{
	ReleaseCurrentFile();
}

HRESULT WINAPI CPeFileParser::QueryInterface(
	__in REFIID riid,
	__out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(IPeFile)))
	{
		*ppvObject = static_cast<IPeFile*>(this);
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, __uuidof(IFsStream)))
	{
		if (m_stream == NULL)
		{
			return E_NOT_SET;
		}
		*ppvObject = static_cast<IFsStream*>(m_stream);
		m_stream->AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

HRESULT WINAPI CPeFileParser::GetDosHeader(__out IMAGE_DOS_HEADER *dosHeader)
{
	if (dosHeader == NULL) return E_INVALIDARG;
	*dosHeader = m_dosHeader;
	return S_OK;
}

HRESULT WINAPI CPeFileParser::GetPEHeader(__out IMAGE_NT_HEADERS32 *peHeader)
{
	if (peHeader == NULL) return E_INVALIDARG;
	*peHeader = m_peHeader;
	return S_OK;
}

HRESULT WINAPI CPeFileParser::GetSectionHeader(
	__in UINT sectionIndex,
	__out IMAGE_SECTION_HEADER *sectionHeader)
{
	if (sectionHeader == NULL) return E_INVALIDARG;
	if (sectionIndex >= m_SectionCount) return E_NOT_SET;

	memcpy(sectionHeader, &m_SectionTable[sectionIndex], sizeof(IMAGE_SECTION_HEADER));
	return S_OK;
}

UINT WINAPI CPeFileParser::GetSectionCount(void)
{
	return m_SectionCount;
}

HRESULT WINAPI CPeFileParser::RvaToFileOffset(__in UINT rva, __out UINT *fileOffset)
{
	if (fileOffset == NULL) return E_INVALIDARG;
	if (m_SectionTable == NULL || m_SectionCount == 0) return E_NOT_SET;

	UINT	i;
	for (i = 0; i < m_SectionCount; i++)
	{
		if (rva >= m_SectionTable[i].VirtualAddress &&
			rva < (m_SectionTable[i].VirtualAddress + m_SectionTable[i].Misc.VirtualSize))
		{
			*fileOffset = m_SectionTable[i].PointerToRawData + (rva - m_SectionTable[i].VirtualAddress);
			return S_OK;
		}
	}

	return E_NOT_SET;
}

HRESULT WINAPI CPeFileParser::VaToFileOffset(__in UINT va, __out UINT *fileOffset)
{
	if (fileOffset == NULL) return E_INVALIDARG;

	if (va < m_peHeader.OptionalHeader.ImageBase) return E_INVALIDARG;

	return RvaToFileOffset(va - m_peHeader.OptionalHeader.ImageBase, fileOffset);
}

HRESULT WINAPI CPeFileParser::FileOffsetToRva(__in UINT fileOffset, __out UINT *rva)
{
	if (rva == NULL) return E_INVALIDARG;
	if (m_SectionCount == 0 || m_SectionTable == NULL) return E_NOT_SET;

	for (UINT i = 0; i < m_SectionCount; i++)
	{
		if (fileOffset >= m_SectionTable[i].PointerToRawData &&
			fileOffset < (m_SectionTable[i].PointerToRawData + m_SectionTable[i].SizeOfRawData))
		{
			*rva = m_SectionTable[i].VirtualAddress + (fileOffset - m_SectionTable[i].PointerToRawData);
			return S_OK;
		}
	}

	return E_NOT_SET;
}

HRESULT WINAPI CPeFileParser::FileOffsetToVa(__in UINT fileOffset, __out UINT *va)
{
	HRESULT	hr;

	hr = FileOffsetToRva(fileOffset, va);
	if (FAILED(hr)) return hr;
	*va += m_peHeader.OptionalHeader.ImageBase;
	return hr;
}

HRESULT WINAPI CPeFileParser::ReadSectionData(
	__in UINT sectionIndex,
	__out_bcount(maxReadSize) LPVOID buffer,
	__in ULONG maxReadSize,
	__out_opt ULONG *bytesRead)
{
	if (m_stream == NULL) return E_NOT_VALID_STATE;
	ULARGE_INTEGER pos;
	LARGE_INTEGER distanceToMove = {};
	distanceToMove.LowPart = m_SectionTable[sectionIndex].PointerToRawData;
	HRESULT hr = m_stream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin);
	if (FAILED(hr)) return hr;
	ULONG nSize = maxReadSize < m_SectionTable[sectionIndex].SizeOfRawData ? maxReadSize : m_SectionTable[sectionIndex].SizeOfRawData;
	return m_stream->Read(buffer, nSize, bytesRead);
}

HRESULT WINAPI CPeFileParser::ReadEPSectionData(
	__out_bcount(maxReadSize) LPVOID buffer,
	__in ULONG maxReadSize,
	__out_opt ULONG *bytesRead)
{
	UINT sectionIndex;
	HRESULT hr = FindSectionByRva(m_peHeader.OptionalHeader.AddressOfEntryPoint, &sectionIndex);
	if (FAILED(hr)) return hr;
	return ReadSectionData(sectionIndex, buffer, maxReadSize, bytesRead);
}

HRESULT WINAPI CPeFileParser::FindSectionByRva(__in UINT rva, __out UINT *sectionIndex)
{
	if (sectionIndex == NULL) return E_INVALIDARG;
	if (m_SectionTable == NULL || m_SectionCount == 0) return E_NOT_SET;

	UINT	i;
	for (i = 0; i < m_SectionCount; i++)
	{
		if (rva >= m_SectionTable[i].VirtualAddress &&
			rva < (m_SectionTable[i].VirtualAddress + m_SectionTable[i].Misc.VirtualSize))
		{
			*sectionIndex = i;
			return S_OK;
		}
	}

	return E_NOT_SET;
}

HRESULT WINAPI CPeFileParser::FindSectionByVa(__in UINT va, __out UINT *sectionIndex)
{
	if (va < m_peHeader.OptionalHeader.ImageBase) return E_INVALIDARG;
	return FindSectionByRva(va - m_peHeader.OptionalHeader.ImageBase, sectionIndex);
}

HRESULT WINAPI CPeFileParser::FindSectionByFileOffset(__in UINT fileOffset, __out UINT *sectionIndex)
{
	HRESULT hr;
	UINT rva;
	hr = FileOffsetToRva(fileOffset, &rva);
	if (FAILED(hr)) return hr;
	return FindSectionByRva(rva, sectionIndex);
}

HRESULT WINAPI CPeFileParser::ReadEntryPointData(
	__out_bcount(maxReadSize) LPVOID buffer,
	__in ULONG maxReadSize,
	__out_opt ULONG *bytesRead)
{
	if (m_stream == NULL) return E_NOT_VALID_STATE;

	UINT epFileOffset = 0;
	HRESULT hr = RvaToFileOffset(m_peHeader.OptionalHeader.AddressOfEntryPoint, &epFileOffset);
	if (FAILED(hr)) return hr;

	ULARGE_INTEGER pos;
	LARGE_INTEGER distanceToMove = {};
	distanceToMove.LowPart = epFileOffset;
	hr = m_stream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin);
	if (FAILED(hr)) return hr;
	return m_stream->Read(buffer, maxReadSize, bytesRead);
}

HRESULT WINAPI CPeFileParser::CheckType(__in IVirtualFs* fsFile, __out BOOL *typeMatched)
{
	if (fsFile == NULL || typeMatched == NULL) return E_INVALIDARG;

	IFsStream *fileStream = NULL;
	HRESULT hr;
	BOOL    fileOpened = FALSE;

	if (fsFile == m_file)
	{
		*typeMatched = m_typeMatched;
		return S_OK;
	}
	else
	{
		ReleaseCurrentFile();
	}

	m_typeMatched = FALSE;
	// Try to open file if file was not opened
	hr = fsFile->IsOpened(&fileOpened);
	if (FAILED(hr)) return hr;

	if (!fileOpened)
	{
		hr = fsFile->ReCreate();
		if (FAILED(hr)) return hr;
	}

	if (FAILED(fsFile->QueryInterface(__uuidof(IFsStream), (LPVOID*)&fileStream)))
	{
		if (!fileOpened) fsFile->Close();
		return E_NOT_VALID_STATE;
	}

	// Parse headers
	if (!ParsePEHeader(fileStream))
	{
		m_typeMatched = FALSE;
		*typeMatched = m_typeMatched;
		fileStream->Release();
		if (!fileOpened) fsFile->Close();
		return S_OK;
	}

	// Parse other PE parts
	InitSectionTable(fileStream);

	fsFile->QueryInterface(__uuidof(IFsStream), (LPVOID*)&m_stream);
	fsFile->QueryInterface(__uuidof(IVirtualFs), (LPVOID*)&m_file);

	fileStream->Release();
	m_typeMatched = TRUE;
	*typeMatched = m_typeMatched;
	return S_OK;
}

HRESULT WINAPI CPeFileParser::Truncate(__in UINT va, __in_opt BOOL padding /*= FALSE*/)
{
	HRESULT hr;
	if (va < m_peHeader.OptionalHeader.ImageBase) return E_INVALIDARG;
	UINT rva = va - m_peHeader.OptionalHeader.ImageBase;
	UINT vrSection = 0;

	for (UINT i = 0; i < m_SectionCount; ++i)
	{
		if (rva == m_SectionTable[i].VirtualAddress)
			return TruncateSectionUntilEndOfFile(i);
		else if (rva > m_SectionTable[i].VirtualAddress &&
			rva < m_SectionTable[i].VirtualAddress + m_SectionTable[i].Misc.VirtualSize)
		{
			vrSection = i;
			break;
		}
	}

	if (vrSection != 0)
	{
		ULARGE_INTEGER fileSize;
		IFsAttribute * attr = NULL;
		if (FAILED(hr = m_file->QueryInterface(__uuidof(IFsAttribute), (LPVOID*)&attr)))
			return hr;
		hr = attr->Size(&fileSize);
		attr->Release();
		attr = NULL;

		UINT offset;
		if (FAILED(RvaToFileOffset(rva, &offset))) return S_OK;

		if ((padding == FALSE) && ((ULONGLONG)(m_SectionTable[m_SectionCount - 1].PointerToRawData + m_SectionTable[m_SectionCount - 1].SizeOfRawData) == fileSize.QuadPart))
		{
			DWORD diffVirtualSize = Align(m_SectionTable[vrSection].VirtualAddress + m_SectionTable[vrSection].Misc.VirtualSize, m_peHeader.OptionalHeader.SectionAlignment) - Align(rva, m_peHeader.OptionalHeader.SectionAlignment);
			DWORD diffSizeOfRawData = Align(m_SectionTable[vrSection].PointerToRawData + m_SectionTable[vrSection].SizeOfRawData, m_peHeader.OptionalHeader.FileAlignment) - Align(offset, m_peHeader.OptionalHeader.FileAlignment);
			m_peHeader.OptionalHeader.SizeOfImage -= diffVirtualSize;
			if (TEST_FLAG(m_SectionTable[vrSection].Characteristics, IMAGE_SCN_MEM_EXECUTE))
				m_peHeader.OptionalHeader.SizeOfCode -= diffVirtualSize;
			m_SectionTable[vrSection].Misc.VirtualSize -= diffVirtualSize;
			m_SectionTable[vrSection].SizeOfRawData -= diffSizeOfRawData;
			if (FAILED(hr = FlushPeHeader()))
				return hr;
			LARGE_INTEGER distanceToMove;

			distanceToMove.QuadPart = m_SectionTable[vrSection].PointerToRawData + m_SectionTable[vrSection].SizeOfRawData;
			if (FAILED(hr = m_stream->Seek(NULL, distanceToMove, IFsStream::FsStreamBegin)))
			{
				if (FAILED(hr)) return hr;
				return E_FAIL;
			}
			return m_stream->Shrink();
		}
		else
		{
			//padding
			DWORD diffSizeOfRawData = Align(m_SectionTable[vrSection].PointerToRawData + m_SectionTable[vrSection].SizeOfRawData, m_peHeader.OptionalHeader.FileAlignment) - Align(offset, m_peHeader.OptionalHeader.FileAlignment);
			LARGE_INTEGER distanceToMove = {};
			distanceToMove.QuadPart = offset;
			unsigned char tmp[1024];
			memset(tmp, 0xc3, sizeof(tmp));
			m_stream->Seek(NULL, distanceToMove, IFsStream::FsStreamBegin);
			while (diffSizeOfRawData > 0)
			{
				ULONG writtenSize;
				ULONG writeSize = (diffSizeOfRawData < sizeof(tmp)) ? diffSizeOfRawData : sizeof(tmp);
				if (FAILED(hr = m_stream->Write(tmp, writeSize, &writtenSize)))
					return hr;
				diffSizeOfRawData -= writtenSize;
			}
			return S_OK;
		}
	}

	return E_NOT_VALID_STATE;
}

bool CPeFileParser::ValidatePeHeader(void)
{
	if (m_peHeader.Signature != IMAGE_NT_SIGNATURE)  return false;
	//FileHeader
	if (m_peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_I386) return false;
	if (m_peHeader.FileHeader.NumberOfSections > MAX_SECTION_COUNT) return false;
	if (m_peHeader.FileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER32)) return false;
	// OptionalHeader
	if (m_peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) return false;

	return true;
}

bool CPeFileParser::ParsePEHeader(__in IFsStream *fsStream)
{
	if (fsStream == NULL) return false;

	ULARGE_INTEGER	pos = { 0, 0 };
	LARGE_INTEGER   distanceToMove = { 0, 0 };
	ULONG			readSize;

	// Parse DOS header
	if (FAILED(fsStream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin)) ||
		FAILED(fsStream->Read(&m_dosHeader, sizeof(IMAGE_DOS_HEADER), &readSize)) ||
		readSize != sizeof(IMAGE_DOS_HEADER) ||
		m_dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
	{
		ZeroMemory(&m_dosHeader, sizeof(m_dosHeader));
		return false;
	}

	// Parse PE header
	distanceToMove.LowPart = m_lfanew = m_dosHeader.e_lfanew;
	distanceToMove.HighPart = 0;
	if (FAILED(fsStream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin)) ||
		FAILED(fsStream->Read(&m_peHeader, sizeof(IMAGE_NT_HEADERS32), &readSize)) ||
		readSize != sizeof(IMAGE_NT_HEADERS32) ||
		!ValidatePeHeader())
	{
		ZeroMemory(&m_peHeader, sizeof(m_peHeader));
		return false;
	}

	return true;
}

void CPeFileParser::InitSectionTable(__in IFsStream *fsStream)
{
	if (fsStream == NULL) return;

	// Seek to the beginning of the section table
	ULARGE_INTEGER	pos = { 0, 0 };
	LARGE_INTEGER   distanceToMove = { 0, 0 };
	ULONG			bufSize, readSize;
	ULONG			maxSectionCnt;
	BYTE			*section;

	distanceToMove.LowPart = m_dosHeader.e_lfanew +
		FIELD_OFFSET(IMAGE_NT_HEADERS32, OptionalHeader) +
		m_peHeader.FileHeader.SizeOfOptionalHeader;

	if (FAILED(fsStream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin))) return;

	// Allocate buffer for section table
	maxSectionCnt = m_peHeader.FileHeader.NumberOfSections;
	if (maxSectionCnt > MAX_SECTION_COUNT) maxSectionCnt = MAX_SECTION_COUNT;
	bufSize = IMAGE_SIZEOF_SECTION_HEADER * maxSectionCnt;
	section = new BYTE[bufSize];
	if (section == NULL) return;
	ZeroMemory(section, bufSize);

	// Try to read a whole section table
	if (FAILED(fsStream->Read(section, bufSize, &readSize)))
	{
		delete[] section;
		return;
	}

	ParseSectionTable(section, maxSectionCnt);
	delete[] section;
}

void CPeFileParser::ParseSectionTable(__in const BYTE *sectionData, __in ULONG maxSectionCount)
{
	UINT	cnt = 0;
	BYTE	*buffer;
	ULONG	bufferSize;
	IMAGE_SECTION_HEADER	nullHeader = {};

	// Find the correct section count
	if (sectionData == NULL) return;
	while (cnt < maxSectionCount)
	{
		if (memcmp(&nullHeader,
			sectionData + cnt * IMAGE_SIZEOF_SECTION_HEADER,
			IMAGE_SIZEOF_SECTION_HEADER) == 0)
		{
			// Null section
			break;
		}
		cnt++;
	}
	if (cnt == 0) return;

	// Allocate buffer to store the section table data
	bufferSize = cnt * IMAGE_SIZEOF_SECTION_HEADER;
	buffer = new BYTE[bufferSize];
	if (buffer == NULL) return;

	// Copy section table data
	m_SectionTable = (IMAGE_SECTION_HEADER*)buffer;
	memcpy(m_SectionTable, sectionData, bufferSize);
	m_OriginalSectionCount = m_SectionCount = cnt;
}

void WINAPI CPeFileParser::ReleaseCurrentFile(void)
{
	ZeroMemory(&m_dosHeader, sizeof(m_dosHeader));
	ZeroMemory(&m_peHeader, sizeof(m_peHeader));
	m_SectionCount = 0;
	delete[]((BYTE*)m_SectionTable);
	m_SectionTable = NULL;
	m_typeMatched = FALSE;

	if (m_stream)
	{
		m_stream->Release();
		m_stream = NULL;
	}

	if (m_file)
	{
		m_file->Release();
		m_file = NULL;
	}
}

DWORD CPeFileParser::Align(__in DWORD n, __in DWORD a)
{
	return (n / a + ((n % a) ? 1 : 0)) * a;
}

HRESULT CPeFileParser::FlushPeHeader(void)
{
	HRESULT hr;
	LARGE_INTEGER distanceToMove;
	ULONG writtenSize;

	distanceToMove.QuadPart = m_lfanew;
	if (FAILED(hr = m_stream->Seek(NULL, distanceToMove, IFsStream::FsStreamBegin)) ||
		FAILED(hr = m_stream->Write(&m_peHeader, sizeof(IMAGE_NT_HEADERS32), &writtenSize)) ||
		writtenSize != sizeof(IMAGE_NT_HEADERS32))
	{
		if (FAILED(hr)) return hr;
		return E_FAIL;
	}
	hr = m_stream->Write(m_SectionTable, IMAGE_SIZEOF_SECTION_HEADER * m_OriginalSectionCount, &writtenSize);
	if (writtenSize != IMAGE_SIZEOF_SECTION_HEADER * m_OriginalSectionCount)
		return E_FAIL;
	return hr;
}

HRESULT WINAPI CPeFileParser::SetVaToEntryPoint(__in UINT va)
{
	if (va < m_peHeader.OptionalHeader.ImageBase) return E_INVALIDARG;
	return SetRvaToEntryPoint(va - m_peHeader.OptionalHeader.ImageBase);
}

HRESULT WINAPI CPeFileParser::SetRvaToEntryPoint(__in UINT rva)
{
	HRESULT hr;
	LARGE_INTEGER distanceToMove;
	ULONG writtenSize;
	ULARGE_INTEGER pos;
	distanceToMove.QuadPart = m_lfanew;
	hr = m_stream->Seek(&pos, distanceToMove, IFsStream::FsStreamBegin);
	if (FAILED(hr))return hr;
	m_peHeader.OptionalHeader.AddressOfEntryPoint = rva;
	if (FAILED(hr = m_stream->Write(&m_peHeader, sizeof(IMAGE_NT_HEADERS32), &writtenSize)) ||
		writtenSize != sizeof(IMAGE_NT_HEADERS32))
	{
		distanceToMove.QuadPart = (LONGLONG)pos.QuadPart;
		m_stream->Seek(NULL, distanceToMove, IFsStream::FsStreamBegin);
		if (FAILED(hr)) return hr;
		return E_FAIL;
	}
	distanceToMove.QuadPart = (LONGLONG)pos.QuadPart;
	m_stream->Seek(NULL, distanceToMove, IFsStream::FsStreamBegin);
	return hr;
}

HRESULT WINAPI CPeFileParser::TruncateSectionUntilEndOfFile(__in UINT sectionIndex)
{
	HRESULT hr;
	LARGE_INTEGER distanceToMove;
	if (sectionIndex || sectionIndex >= m_SectionCount) return E_INVALIDARG;

	for (UINT i = sectionIndex; i < m_SectionCount; ++i)
	{
		m_peHeader.OptionalHeader.SizeOfImage -= m_SectionTable[i].Misc.VirtualSize;
		if (TEST_FLAG(m_SectionTable[i].Characteristics, IMAGE_SCN_MEM_EXECUTE))
			m_peHeader.OptionalHeader.SizeOfCode -= m_SectionTable[i].Misc.VirtualSize;
		ZeroMemory(&m_SectionTable[i], IMAGE_SIZEOF_SECTION_HEADER);
	}

	m_SectionCount = m_peHeader.FileHeader.NumberOfSections = (WORD)sectionIndex;


	if (FAILED(hr = FlushPeHeader()))
		return hr;

	distanceToMove.QuadPart = m_SectionTable[sectionIndex - 1].PointerToRawData + m_SectionTable[sectionIndex - 1].SizeOfRawData;
	if (FAILED(hr = m_stream->Seek(NULL, distanceToMove, IFsStream::FsStreamBegin)))
	{
		if (FAILED(hr)) return hr;
		return E_FAIL;
	}
	return m_stream->Shrink();
}
