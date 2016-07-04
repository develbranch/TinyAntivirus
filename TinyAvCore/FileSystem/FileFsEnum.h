#pragma once
#include <TinyAvCore.h>

class CFileFsEnum :
	public CRefCount,
	public IFsEnum,
	public IFsEnumObserver
{
protected:
	virtual ~CFileFsEnum();

	std::vector<IFsEnumObserver*> m_Observers;
	std::vector<IFsEnum* >		  m_Archivers;
private:
	HANDLE m_hStop;
public:
	CFileFsEnum();

	DECLARE_REF_COUNT();

	virtual HRESULT WINAPI QueryInterface(__in REFIID riid, __out _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);


	virtual HRESULT WINAPI AddObserver(__in IFsEnumObserver *observer) override;

	virtual HRESULT WINAPI RemoveObserver(__in IFsEnumObserver *observer) override;
	virtual HRESULT WINAPI AddArchiver(__in IFsEnum * archiver) override;
	virtual HRESULT WINAPI RemoveArchiver(__in IFsEnum * archiver) override;
	virtual HRESULT WINAPI Enum(__in IFsEnumContext *context) override;
	virtual void    WINAPI Stop(void) override;
	virtual HRESULT WINAPI OnFileFound(__in IVirtualFs *file, __in IFsEnumContext *context, __in const int currentDepth) override;
	virtual void WINAPI OnError(__in DWORD dwErrorCode, __in_opt LPCWSTR lpMessage = NULL) override;

private:
	virtual HRESULT WINAPI IsFileTooLarge(__in IVirtualFs * container, __in LPCWSTR fileName, __in IFsEnumContext *context, __out BOOL* over);
	virtual HRESULT WINAPI IsFileTooLarge(__in IVirtualFs * file, __in IFsEnumContext *context, __out BOOL* over);
	virtual HRESULT WINAPI OnEnumEntryFound(__in IVirtualFs * container, __in LPCWSTR fileName, __in IFsEnumContext *context, __in int currentDepth);
	virtual StringW MakePath(__in LPCWSTR str1, __in  LPCWSTR str2);
	HRESULT CheckDeferredDeletion(__in IVirtualFs * container, __in IVirtualFs * file);

protected:
	virtual void WINAPI InitArchiveObservers(void);
	virtual void WINAPI EnumByArchivers(__in IVirtualFs *file, __in IFsEnumContext *context, __in const int depth, __in const int depthInArchive);
	virtual void WINAPI CleanupArchiveObservers(void);
	virtual BOOL WINAPI TestFilePath(__in LPCWSTR lpFileName);

	HANDLE	m_findHandle;
	WIN32_FIND_DATAW m_wfd;
	virtual BOOL WINAPI EnumInit(void);
	virtual BOOL WINAPI EnumFirstFile(__in LPCWSTR lpFileName);
	virtual BOOL WINAPI EnumNextFile(void);
	virtual void WINAPI EnumClose(void);
};

