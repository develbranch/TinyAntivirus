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
		IsEqualIID(riid, __uuidof(IFileType)) ||
		IsEqualIID(riid, __uuidof(IPeFile)))
	{
		*ppvObject = static_cast<IPeFile*>(this);
		this->AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, __uuidof(IFsStream)))
	{
		if (m_stream)
		{
			*ppvObject = static_cast<IFsStream*>(m_stream);
			m_stream->AddRef();
			return S_OK;
		}
	}
	else if (IsEqualIID(riid, __uuidof(IVirtualFs)))
	{
		if (m_file)
		{
			*ppvObject = static_cast<IVirtualFs*>(m_file);
			m_file->AddRef();
			return S_OK;
		}
	}
	return E_NOINTERFACE;
}

HRESULT WINAPI CPeFileParser::GetDosHeader(__out_bcount(sizeof(IMAGE_DOS_HEADER)) IMAGE_DOS_HEADER *dosHeader)
{
	if (dosHeader == NULL) return E_INVALIDARG;
	*dosHeader = m_dosHeader;
	return S_OK;
}

HRESULT WINAPI CPeFileParser::GetPEHeader(__out_bcount(sizeof(IMAGE_NT_HEADERS32)) IMAGE_NT_HEADERS32 *peHeader)
{
	if (peHeader == NULL) return E_INVALIDARG;
	*peHeader = m_peHeader;
	return S_OK;
}

HRESULT WINAPI CPeFileParser::GetSectionHeader(
	__in UINT sectionIndex,
	__out_bcount(IMAGE_SIZEOF_SECTION_HEADER) IMAGE_SECTION_HEADER *sectionHeader)
{
	if (sectionHeader == NULL) return E_INVALIDARG;
	if (sectionIndex >= m_SectionCount) return E_NOT_SET;

	memcpy(sectionHeader, &m_SectionTable[sectionIndex], IMAGE_SIZEOF_SECTION_HEADER);
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

	LARGE_INTEGER offset = {};
	offset.LowPart = epFileOffset;
	return m_stream->ReadAt(offset, IFsStream::FsStreamBegin, buffer, maxReadSize, bytesRead);
}

HRESULT WINAPI CPeFileParser::CheckType(__in IVirtualFs* fsFile, __out BOOL *typeMatched)
{
	if (fsFile == NULL || typeMatched == NULL) return E_INVALIDARG;

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

	// Parse headers
	if (!ParsePEHeader(fsFile))
	{
		m_typeMatched = FALSE;
		*typeMatched = m_typeMatched;
		if (!fileOpened) fsFile->Close();
		return S_OK;
	}

	fsFile->QueryInterface(__uuidof(IFsStream), (LPVOID*)&m_stream);
	fsFile->QueryInterface(__uuidof(IVirtualFs), (LPVOID*)&m_file);

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
			DWORD diffVirtualSize = SectionAlign(m_SectionTable[vrSection].VirtualAddress + m_SectionTable[vrSection].Misc.VirtualSize, m_peHeader.OptionalHeader.SectionAlignment) - SectionAlign(rva, m_peHeader.OptionalHeader.SectionAlignment);
			DWORD diffSizeOfRawData = SectionAlign(m_SectionTable[vrSection].PointerToRawData + m_SectionTable[vrSection].SizeOfRawData, m_peHeader.OptionalHeader.FileAlignment) - SectionAlign(offset, m_peHeader.OptionalHeader.FileAlignment);
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
			DWORD diffSizeOfRawData = SectionAlign(m_SectionTable[vrSection].PointerToRawData + m_SectionTable[vrSection].SizeOfRawData, m_peHeader.OptionalHeader.FileAlignment) - SectionAlign(offset, m_peHeader.OptionalHeader.FileAlignment);
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

bool CPeFileParser::ParsePEHeader(__in IVirtualFs* fsFile)
{
	HRESULT hr;
	ULARGE_INTEGER fileSize = {};
	IFsAttribute * attribute = NULL;
	ULARGE_INTEGER	pos = { 0, 0 };
	LARGE_INTEGER   offset = { 0, 0 };
	ULONG			readSize;
	IFsStream*		fsStream = NULL;

	if (fsFile == NULL) return false;

	// get file size
	if (FAILED(fsFile->QueryInterface(__uuidof(IFsAttribute), (LPVOID*)&attribute)))
		return false;
	hr = attribute->Size(&fileSize);
	attribute->Release();
	if (FAILED(hr)) return false;

	// get file stream
	if (FAILED(fsFile->QueryInterface(__uuidof(IFsStream), (LPVOID*)&fsStream)))
		return false;

	// Parse DOS header
	if (FAILED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, &m_dosHeader, sizeof(IMAGE_DOS_HEADER), &readSize)) ||
		readSize != sizeof(IMAGE_DOS_HEADER) ||
		m_dosHeader.e_magic != IMAGE_DOS_SIGNATURE ||
		m_dosHeader.e_lfanew <= 0 || //malformed DOS header
		((ULONGLONG)m_dosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS32)) >= fileSize.QuadPart
		)
	{
		ZeroMemory(&m_dosHeader, sizeof(m_dosHeader));
		fsStream->Release();
		return false;
	}

	// Parse PE header
	offset.LowPart = m_lfanew = m_dosHeader.e_lfanew;
	offset.HighPart = 0;
	if (FAILED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, &m_peHeader, sizeof(IMAGE_NT_HEADERS32), &readSize)) ||
		readSize != sizeof(IMAGE_NT_HEADERS32))
	{
		ZeroMemory(&m_peHeader, sizeof(m_peHeader));
		fsStream->Release();
		return false;
	}

	// check for malformed PE header
	bool res = ValidatePeHeader(fsStream);
	if (!res)
	{
		ZeroMemory(&m_peHeader, sizeof(m_peHeader));
	}

	fsStream->Release();
	return res;
}

bool CPeFileParser::ValidatePeHeader(__in IFsStream *fsStream)
{
	if (m_peHeader.Signature != IMAGE_NT_SIGNATURE)  return false;
	//FileHeader
	if (m_peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ) return false;
	/*
	Quote from PECOFF documentation: "Note that the Windows loader limits the number of sections to 96."
	From Windows Vista, the range from zero to the maximum value 0xFFFF.*/

	if (m_peHeader.FileHeader.NumberOfSections == 0 || m_peHeader.FileHeader.NumberOfSections > MAX_SECTION_COUNT) return false;
	if (m_peHeader.FileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER32)) return false;
	// OptionalHeader
	if (m_peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) return false;
	if (m_peHeader.OptionalHeader.FileAlignment == 0) return false;
	if (m_peHeader.OptionalHeader.SectionAlignment == 0) return false;

	// check section alignment
	if ((m_peHeader.OptionalHeader.SectionAlignment & (m_peHeader.OptionalHeader.SectionAlignment - 1)) != 0)
		return false;

	// check file alignment
	if ((m_peHeader.OptionalHeader.FileAlignment & (m_peHeader.OptionalHeader.FileAlignment - 1)) != 0)
		return false;

	if (m_peHeader.OptionalHeader.SectionAlignment < m_peHeader.OptionalHeader.FileAlignment) return false;
	if (m_peHeader.OptionalHeader.SectionAlignment % m_peHeader.OptionalHeader.FileAlignment) return false;
	if (m_peHeader.OptionalHeader.SizeOfImage == 0) return false;
	if (m_peHeader.OptionalHeader.SizeOfImage % m_peHeader.OptionalHeader.SectionAlignment) return false;
	if (m_peHeader.OptionalHeader.ImageBase == 0 && m_peHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == 0) return false;
	
	if (
		(!TEST_FLAG(m_peHeader.FileHeader.Characteristics, IMAGE_FILE_LARGE_ADDRESS_AWARE))&& 
				(m_peHeader.OptionalHeader.ImageBase && m_peHeader.OptionalHeader.ImageBase + m_peHeader.OptionalHeader.SizeOfImage >=0x80000000 )
		)
		return false;

	if (
		(TEST_FLAG(m_peHeader.FileHeader.Characteristics, IMAGE_FILE_LARGE_ADDRESS_AWARE)) &&
		(m_peHeader.OptionalHeader.ImageBase && m_peHeader.OptionalHeader.ImageBase + m_peHeader.OptionalHeader.SizeOfImage >= 0xC0000000)
		)
		return false;

	if (m_peHeader.OptionalHeader.SizeOfStackCommit == 0 || m_peHeader.OptionalHeader.SizeOfStackReserve == 0) return false;

	// Parse other PE parts
	return InitSectionTable(fsStream);
}

bool CPeFileParser::InitSectionTable(__in IFsStream *fsStream)
{
	if (fsStream == NULL) return false;

	// Seek to the beginning of the section table
	ULARGE_INTEGER	pos = { 0, 0 };
	LARGE_INTEGER   offset = { 0, 0 };
	ULONG			bufSize, readSize;
	ULONG			maxSectionCnt;
	BYTE			*section;

	offset.LowPart = m_dosHeader.e_lfanew +
		FIELD_OFFSET(IMAGE_NT_HEADERS32, OptionalHeader) +
		m_peHeader.FileHeader.SizeOfOptionalHeader;

	// Allocate buffer for section table
	maxSectionCnt = m_peHeader.FileHeader.NumberOfSections;
	if (maxSectionCnt > MAX_SECTION_COUNT) maxSectionCnt = MAX_SECTION_COUNT;
	bufSize = IMAGE_SIZEOF_SECTION_HEADER * maxSectionCnt;
	section = new BYTE[bufSize];
	if (section == NULL) return false;
	ZeroMemory(section, bufSize);

	// Try to read a whole section table
	if (FAILED(fsStream->ReadAt(offset, IFsStream::FsStreamBegin, section, bufSize, &readSize)))
	{
		delete[] section;
		return false;
	}

	bool res = ParseSectionTable(section, maxSectionCnt);
	delete[] section;
	return res;
}

bool CPeFileParser::ParseSectionTable(__in const BYTE *sectionData, __in ULONG maxSectionCount)
{
	UINT	cnt = 0;
	BYTE	*buffer;
	ULONG	bufferSize;
	IMAGE_SECTION_HEADER	nullHeader = {};
	ULONG TotalVirtualSize = 0;

	// Find the correct section count
	if (sectionData == NULL)  return false;
	while (cnt < maxSectionCount)
	{
		if (memcmp(&nullHeader,
			sectionData + cnt * IMAGE_SIZEOF_SECTION_HEADER,
			IMAGE_SIZEOF_SECTION_HEADER) == 0)
		{
			// Null section
			break;
		}
		IMAGE_SECTION_HEADER* section = (IMAGE_SECTION_HEADER*)(sectionData + cnt * IMAGE_SIZEOF_SECTION_HEADER);
		TotalVirtualSize = SectionAlign(section->Misc.VirtualSize, m_peHeader.OptionalHeader.SectionAlignment) + section->VirtualAddress ;
		cnt++;
	}
	if (cnt == 0)  return false;
	if (TotalVirtualSize != m_peHeader.OptionalHeader.SizeOfImage) return false;

	// Allocate buffer to store the section table data
	bufferSize = cnt * IMAGE_SIZEOF_SECTION_HEADER;
	buffer = new BYTE[bufferSize];
	if (buffer == NULL)  return false;

	// Copy section table data
	m_SectionTable = (IMAGE_SECTION_HEADER*)buffer;
	memcpy(m_SectionTable, sectionData, bufferSize);
	m_OriginalSectionCount = m_SectionCount = cnt;
	return true;
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

DWORD CPeFileParser::SectionAlign(__in DWORD n, __in DWORD a)
{
	//return (n / a + ((n % a) ? 1 : 0)) * a;
	return (n + (a - 1)) & ~(a - 1);
}

DWORD CPeFileParser::FileAlign(__in DWORD n, __in DWORD a)
{
	return n & ~(a - 1);
}

HRESULT CPeFileParser::FlushPeHeader(void)
{
	HRESULT hr;
	LARGE_INTEGER fileOffset;
	ULONG writtenSize;

	fileOffset.QuadPart = m_lfanew;
	if (FAILED(hr = m_stream->WriteAt(fileOffset, IFsStream::FsStreamBegin, &m_peHeader, sizeof(IMAGE_NT_HEADERS32), &writtenSize)) ||
		writtenSize != sizeof(IMAGE_NT_HEADERS32))
	{
		if (FAILED(hr)) return hr;
		return E_FAIL;
	}
	hr = m_stream->Write((LPBYTE)m_SectionTable, IMAGE_SIZEOF_SECTION_HEADER * m_OriginalSectionCount, &writtenSize);
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
	LARGE_INTEGER headerOffset;
	ULONG writtenSize;
	ULARGE_INTEGER pos;
	headerOffset.QuadPart = m_lfanew;

	m_peHeader.OptionalHeader.AddressOfEntryPoint = rva;
	hr = m_stream->Seek(&pos, headerOffset, IFsStream::FsStreamBegin);
	if (FAILED(hr)) return hr;

	if (FAILED(hr = m_stream->Write(&m_peHeader, sizeof(IMAGE_NT_HEADERS32), &writtenSize)) ||
		writtenSize != sizeof(IMAGE_NT_HEADERS32))
	{
		headerOffset.QuadPart = (LONGLONG)pos.QuadPart;
		m_stream->Seek(NULL, headerOffset, IFsStream::FsStreamBegin);
		if (FAILED(hr)) return hr;
		return E_FAIL;
	}
	headerOffset.QuadPart = (LONGLONG)pos.QuadPart;
	m_stream->Seek(NULL, headerOffset, IFsStream::FsStreamBegin);
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
