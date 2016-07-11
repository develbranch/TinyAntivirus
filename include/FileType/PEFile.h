#pragma once

#include "../TinyAvBase.h"
#include "FileType.h"

#define MAX_SECTION_COUNT	 (96)
#define MAX_PE_HEADER_SIZE (4096)

// -----------------------------
// Interface for 32-bit PE file
// -----------------------------
MIDL_INTERFACE("34D42F84-0224-4195-AD90-57767B4E855F")
IPeFile : public IFileType
{
	BEGIN_INTERFACE

public:

	/* Retrieve DOS header
	@dosHeader: a pointer to an variable storing IMAGE_DOS_HEADER header
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI GetDosHeader(__out_bcount(sizeof(IMAGE_DOS_HEADER)) IMAGE_DOS_HEADER *dosHeader) = 0;
	
	/* Retrieve PE header
	@dosHeader: a pointer to an variable storing IMAGE_NT_HEADERS32 header
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI GetPEHeader(__out_bcount(sizeof(IMAGE_NT_HEADERS32)) IMAGE_NT_HEADERS32 *peHeader) = 0;
	
	/* Retrieve section header
	@sectionIndex: section index
	@sectionHeader: a pointer to an variable storing IMAGE_SECTION_HEADER header
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI GetSectionHeader(__in UINT sectionIndex, __out_bcount(IMAGE_SIZEOF_SECTION_HEADER) IMAGE_SECTION_HEADER *sectionHeader) = 0;
	
	// Retrieve the number of section in header
	//@return: number of section.
	virtual UINT WINAPI GetSectionCount(void) = 0;

	/* Convert RVA to file offset
	@rva: relative virtual address 
	@fileOffset: a pointer to an variable storing file offset
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI RvaToFileOffset(__in UINT rva, __out UINT *fileOffset) = 0;

	/* Convert VA to file offset
	@va: virtual address
	@fileOffset: a pointer to an variable storing file offset
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI VaToFileOffset(__in UINT va, __out UINT *fileOffset) = 0;
	
	/* Convert file offset to RVA
	@fileOffset: an address is offset in file
	@rva: a pointer to an variable storing relative virtual address
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI FileOffsetToRva(__in UINT fileOffset, __out UINT *rva) = 0;
	
	/* Convert file offset to VA
	@fileOffset: an address is offset in file
	@va: a pointer to an variable storing virtual address
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI FileOffsetToVa(__in UINT fileOffset, __out UINT *va) = 0;
	
	/* Read raw data of the given section
	@sectionIndex: section number
	@buffer:   pointer to a variable containing data copied from file.
	@maxReadSize:   size of data to copy.
	@bytesRead: a pointer to an variable storing byte read
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI ReadSectionData(__in UINT sectionIndex, __out_bcount(maxReadSize) LPVOID buffer, __in ULONG maxReadSize, __out_opt ULONG *bytesRead) = 0;

	/* Read raw data of the section containing Entry-Point
	@buffer: pointer to a variable containing data copied from file.
	@maxReadSize: size of data to copy.
	@bytesRead: a pointer to an variable storing byte read
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI ReadEPSectionData(__out_bcount(maxReadSize) LPVOID buffer, __in ULONG maxReadSize, __out_opt ULONG *bytesRead) = 0;

	/* Find section containing the given RVA
	@rva: relative virtual address 
	@sectionIndex: a pointer to an variable storing section index
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI FindSectionByRva(__in UINT rva, __out UINT *sectionIndex) = 0;
	
	/* Find section containing the given VA
	@va: virtual address
	@sectionIndex: a pointer to an variable storing section index
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI FindSectionByVa(__in UINT va, __out UINT *sectionIndex) = 0;

	/* Find section containing the given raw file offset
	@fileOffset: file offset
	@sectionIndex: a pointer to an variable storing section index
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI FindSectionByFileOffset(__in UINT fileOffset, __out UINT *sectionIndex) = 0;

	/* Read buffer starting at the Entry-Point
	@buffer: pointer to a variable containing data copied from file.
	@maxReadSize: size of data to copy.
	@bytesRead: a pointer to an variable storing byte read
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI ReadEntryPointData(__out_bcount(maxReadSize) LPVOID buffer, __in ULONG maxReadSize, __out_opt ULONG *bytesRead) = 0;
	
	/* Set new Address of Entry Point
	@va: Address of new entry point.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI SetVaToEntryPoint(__in UINT va) = 0;

	/* Set new Address of Entry Point
	@rva: relative address of Address of new entry point.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI SetRvaToEntryPoint(__in UINT rva) = 0;

	/* shrink file from current section to end of file 
	@sectionIndex: relative address of Address of new entry point.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI TruncateSectionUntilEndOfFile(__in UINT sectionIndex) = 0;
	
	/* shrink file to end of file or patch file with 0xC3
	@va: virtual address
	@padding: Indicates whether to shrink or patch file.
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI Truncate(__in UINT va, __in_opt BOOL padding = FALSE) = 0;

	//
	virtual void WINAPI ReleaseCurrentFile(void) =0;
	
	END_INTERFACE
};


// -----------------------------
// Interface for 64-bit PE file
// -----------------------------
MIDL_INTERFACE("80EABAE2-34D4-448F-BF92-235C62A7151A")
IPe64File : public IFileType
{
	BEGIN_INTERFACE

public:

	// Get DOS header
	virtual HRESULT WINAPI GetDosHeader(__out IMAGE_DOS_HEADER *dosHeader) = 0;
	// Get PE header
	virtual HRESULT WINAPI GetPEHeader(__out IMAGE_NT_HEADERS64 *peHeader) = 0;
	// Get section header
	virtual HRESULT WINAPI GetSectionHeader(__in UINT sectionIndex, __out IMAGE_SECTION_HEADER *sectionHeader) = 0;
	// Get section count
	virtual HRESULT WINAPI GetSectionCount(__out UINT64 *sectionCount) = 0;
	// Convert RVA to file offset
	virtual HRESULT WINAPI RvaToFileOffset(__in UINT64 rva, __out UINT *fileOffset) = 0;
	// Convert VA to file offset
	virtual HRESULT WINAPI VaToFileOffset(__in UINT64 va, __out UINT *fileOffset) = 0;
	// Convert file offset to RVA
	virtual HRESULT WINAPI FileOffsetToRva(__in UINT fileOffset, __out UINT64 *rva) = 0;
	// Convert file offset to VA
	virtual HRESULT WINAPI FileOffsetToVa(__in UINT fileOffset, __out UINT64 *va) = 0;
	// Read raw data of the given section
	virtual HRESULT WINAPI ReadSectionData(__in UINT sectionIndex, __out_bcount(maxReadSize) LPVOID buffer, __in ULONG maxReadSize, __out_opt ULONG *bytesRead) = 0;
	// Read raw data of the section containing Entry-Point
	virtual HRESULT WINAPI ReadEPSectionData(__out_bcount(maxReadSize) LPVOID buffer, __in ULONG maxReadSize, __out_opt ULONG *bytesRead) = 0;
	// Find section containing the given RVA
	virtual HRESULT WINAPI FindSectionByRva(__in UINT64 rva, __out UINT *sectionIndex) = 0;
	// Find section containing the given VA
	virtual HRESULT WINAPI FindSectionByVa(__in UINT64 va, __out UINT *sectionIndex) = 0;
	// Find section containing the given raw file offset
	virtual HRESULT WINAPI FindSectionByFileOffset(__in UINT fileOffset, __out UINT *sectionIndex) = 0;
	// Read buffer starting at the Entry-Point
	virtual HRESULT WINAPI ReadEntryPointData(__out_bcount(maxReadSize) LPVOID buffer, __in ULONG maxReadSize, __out_opt ULONG *bytesRead) = 0;
	
	virtual HRESULT WINAPI SetVaToEntryPoint(__in UINT64 va) = 0;
	virtual HRESULT WINAPI SetRvaToEntryPoint(__in UINT64 rva) = 0;
	virtual HRESULT WINAPI TruncateSectionUntilEndOfFile(__in UINT sectionIndex) = 0;
	virtual HRESULT WINAPI Truncate(__in UINT64 va, __in_opt BOOL padding = FALSE) = 0;
	virtual void WINAPI ReleaseCurrentFile(void) = 0;

	END_INTERFACE
};
