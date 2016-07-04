#pragma once
#include <TinyAvCore.h>

class CPeFileParser:
	public CRefCount, 
	public IPeFile
{
private:
	DWORD m_lfanew;
protected:

	IMAGE_DOS_HEADER m_dosHeader;
	IMAGE_NT_HEADERS32 m_peHeader;
	UINT m_SectionCount;
	UINT m_OriginalSectionCount;
	IMAGE_SECTION_HEADER *m_SectionTable;
	BOOL m_typeMatched;
	IVirtualFs * m_file;
	IFsStream *m_stream;
	virtual ~CPeFileParser();

public:
	CPeFileParser();

	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual HRESULT WINAPI GetDosHeader(__out IMAGE_DOS_HEADER *dosHeader) override;

	virtual HRESULT WINAPI GetPEHeader(__out IMAGE_NT_HEADERS32 *peHeader) override;

	virtual HRESULT WINAPI GetSectionHeader(__in UINT sectionIndex, __out IMAGE_SECTION_HEADER *sectionHeader) override;

	virtual UINT WINAPI GetSectionCount(void) override;

	virtual HRESULT WINAPI RvaToFileOffset(__in UINT rva, __out UINT *fileOffset) override;

	virtual HRESULT WINAPI VaToFileOffset(__in UINT va, __out UINT *fileOffset) override;

	virtual HRESULT WINAPI FileOffsetToRva(__in UINT fileOffset, __out UINT *rva) override;

	virtual HRESULT WINAPI FileOffsetToVa(__in UINT fileOffset, __out UINT *va) override;

	virtual HRESULT WINAPI ReadSectionData(__in UINT sectionIndex, __out_bcount(maxReadSize) LPVOID buffer, __in ULONG maxReadSize, __out_opt ULONG *bytesRead) override;

	virtual HRESULT WINAPI ReadEPSectionData(__out_bcount(maxReadSize) LPVOID buffer, __in ULONG maxReadSize, __out_opt ULONG *bytesRead) override;

	virtual HRESULT WINAPI FindSectionByRva(__in UINT rva, __out UINT *sectionIndex) override;

	virtual HRESULT WINAPI FindSectionByVa(__in UINT va, __out UINT *sectionIndex) override;

	virtual HRESULT WINAPI FindSectionByFileOffset(__in UINT fileOffset, __out UINT *sectionIndex) override;

	virtual HRESULT WINAPI ReadEntryPointData(__out_bcount(maxReadSize) LPVOID buffer, __in ULONG maxReadSize, __out_opt ULONG *bytesRead) override;
	
	virtual HRESULT WINAPI SetVaToEntryPoint(__in UINT va) override;

	virtual HRESULT WINAPI SetRvaToEntryPoint(__in UINT rva) override;

	virtual HRESULT WINAPI TruncateSectionUntilEndOfFile(__in UINT sectionIndex) override;
	
	// Check for type matching
	virtual HRESULT WINAPI CheckType(__in IVirtualFs* fsFile, __out BOOL *typeMatched) override;

	virtual void WINAPI ReleaseCurrentFile(void) override;

	virtual HRESULT WINAPI Truncate(__in UINT va, __in_opt BOOL padding = FALSE) override;

	// 

private:
	// -----------------------------
	// Internal methods
	// -----------------------------
	bool ValidatePeHeader(void);
	// Parse the PE header
	bool ParsePEHeader(__in IFsStream *fsStream);

	// Initialize the section table
	void InitSectionTable(__in IFsStream *fsStream);
	// Parse the section table
	void ParseSectionTable(__in const BYTE *sectionData, __in ULONG maxSectionCount);

	DWORD Align(__in DWORD n, __in DWORD a);

protected:
	HRESULT FlushPeHeader(void);
};

