#include "Utils.h"
#include <InitGuid.h>
#include "TinyAvCore.h"
#include "Module\ModuleMgrService.h"
#include "Emulator\PeEmulator.h"
#include "FileType\PeFileParser.h"
#include "Scanner\ScanService.h"
#include "FileSystem\FileFsEnumContext.h"
#include "FileSystem\FileFs.h"

StringW AnsiToUnicode(__in StringA * str)
{
	if (str == NULL) return L"";

	int nRequired = MultiByteToWideChar(CP_UTF8, 0, str->c_str(), (int)str->length(), NULL, 0);
	if (nRequired == 0) return L"";

	WCHAR * lpWideChar = new WCHAR[nRequired + 1];
	if (lpWideChar == NULL) return L"";
	nRequired = MultiByteToWideChar(CP_UTF8, 0, str->c_str(), (int)str->length(), lpWideChar, nRequired);
	if (nRequired == 0) return L"";
	lpWideChar[nRequired] = L'\0';
	StringW wstr = lpWideChar;
	delete[]lpWideChar;
	return wstr;
}

StringW AnsiToUnicode(__in StringA& str)
{
	return AnsiToUnicode(&str);
}

StringA UnicodeToAnsi(__in StringW * str)
{
	if (str == NULL) return "";

	int nRequired = WideCharToMultiByte(CP_UTF8, 0, str->c_str(), (int)str->length(), NULL, 0, NULL, NULL);
	if (nRequired == 0) return "";

	CHAR * lpWideChar = new CHAR[nRequired + 1];
	if (lpWideChar == NULL) return "";
	nRequired = WideCharToMultiByte(CP_UTF8, 0, str->c_str(), (int)str->length(), lpWideChar, nRequired, NULL, NULL);
	if (nRequired == 0) return "";
	lpWideChar[nRequired] = '\0';
	StringA wstr = lpWideChar;
	delete[]lpWideChar;
	return wstr;
}

StringA UnicodeToAnsi(__in StringW& str)
{
	return UnicodeToAnsi(&str);
}

HRESULT WINAPI CreateClassObject(__in REFCLSID rclsid, __in DWORD dwClsContext, __in REFIID riid, __out LPVOID *ppv)
{
	UNREFERENCED_PARAMETER(dwClsContext);

	if (ppv == NULL) return E_INVALIDARG;

	if (IsEqualCLSID(rclsid, CLSID_CModuleMgrService) ||
		IsEqualIID(riid, __uuidof(IModuleManager)))
	{
		*ppv = static_cast<IModuleManager*>(new CModuleMgrService());
		return S_OK;
	}

	else if (IsEqualCLSID(rclsid, CLSID_CPeEmulator) ||
		IsEqualIID(riid, __uuidof(IEmulator)))
	{
		*ppv = static_cast<IEmulator*>(new CPeEmulator());
		return S_OK;
	}

	else if (IsEqualCLSID(rclsid, CLSID_CPeFileParser) ||
		IsEqualIID(riid, __uuidof(IPeFile)))
	{
		*ppv = static_cast<IPeFile*>(new CPeFileParser());
		return S_OK;
	}

	else if (IsEqualCLSID(rclsid, CLSID_CScanService) ||
		IsEqualIID(riid, __uuidof(IScanner)))
	{
		*ppv = static_cast<IScanner*>(new CScanService());
		return S_OK;
	}

	else if (IsEqualCLSID(rclsid, CLSID_CFileFsEnumContext) &&
		IsEqualIID(riid, __uuidof(IFsEnumContext)))
	{
		*ppv = static_cast<IFsEnumContext*>(new CFileFsEnumContext());
		return S_OK;
	}

	else if (IsEqualCLSID(rclsid, CLSID_CFileFs) &&
		IsEqualIID(riid, __uuidof(IVirtualFs)))
	{
		*ppv = static_cast<IVirtualFs*>(new CFileFs());
		return S_OK;
	}

	return E_NOINTERFACE;
}