#include "ZipFsEnum.h"
#include "UnzipHelper.h"
#include "../../Utils.h"
#include "ZipFs.h"

CZipFsEnum::CZipFsEnum(void)
{
}

CZipFsEnum::~CZipFsEnum(void)
{
}

HRESULT WINAPI CZipFsEnum::Enum(__in IFsEnumContext *context)
{
	HRESULT hr;
	zlib_filefunc64_def ffunc;
	BSTR lpFileName = NULL;
	IVirtualFs * container = NULL;

	if (context == NULL) return E_INVALIDARG;

	hr = context->GetSearchContainer(&container);
	if (FAILED(hr)) return hr;
	
	FillFunctions64((voidpf)container, &ffunc);
	hr = container->GetFullPath(&lpFileName);
	if (FAILED(hr))
	{
		container->Release();
		return hr;
	}

	unzFile uf = NULL;
	uf = unzOpen2_64(lpFileName, &ffunc);
	SysFreeString(lpFileName);

	if (uf == NULL)
	{
		container->Release();
		return E_FAIL;
	}

	hr = ReadArchiver(container, context, uf);

	unzClose(uf);
	container->Release();
	return hr;
}

HRESULT WINAPI CZipFsEnum::ReadArchiver(__in_opt IVirtualFs * container, __in IFsEnumContext * context, __in void * stream)
{
	char filename_inzip[256] = {};
	unz_file_info64 file_info;
	unz_global_info64 gi;
	int err;
	bool	stopSearch = false;
	ULARGE_INTEGER maxFileSize;
	if (container == NULL || stream == NULL) return E_INVALIDARG;
	HRESULT hr = context->GetMaxFileSize(&maxFileSize);
	if (FAILED(hr)) return hr;

	unzFile uf = (unzFile)stream;

	err = unzGetGlobalInfo64(uf, &gi);
	if (err != UNZ_OK) return E_UNEXPECTED;

	for (ZPOS64_T i = 0; (i < gi.number_entry) && (!stopSearch); i++)
	{
		err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
		if (err != UNZ_OK) break;

		if (file_info.uncompressed_size > (ZPOS64_T)maxFileSize.QuadPart) // skip big-file
			continue;
		
		StringA strName = filename_inzip;
		StringW wstrName = AnsiToUnicode(&strName);

		IVirtualFs * zipFile = static_cast<IVirtualFs*>(new CZipFs());
		if (zipFile)
		{
			if (SUCCEEDED(zipFile->SetContainer(container)) &&
				SUCCEEDED(zipFile->Create(wstrName.c_str(), 0)) &&
				SUCCEEDED(zipFile->ReCreate((void*)uf)))
			{
				IFsAttribute * fsAttrib = NULL;
				if (SUCCEEDED(zipFile->QueryInterface(__uuidof(IFsAttribute), (LPVOID*)&fsAttrib)))
				{
					DWORD dwAttrib;

					if (SUCCEEDED(fsAttrib->Attributes(&dwAttrib)) &&
						(TEST_FLAG(dwAttrib, FILE_ATTRIBUTE_DIRECTORY) == 0))
					{
						int currentDepthInArchive = context->GetDepthInArchive();
						context->SetDepthInArchive(currentDepthInArchive + 1);
						hr = OnFileFound(zipFile, context, context->GetDepth() + 1);
						if (hr == E_ABORT)
						{
							stopSearch = true;
						}
						context->SetDepthInArchive(currentDepthInArchive);
					}

					fsAttrib->Release();
				}
			}

			ULONG flags;
			if (SUCCEEDED(zipFile->GetFlags(&flags)) &&
				TEST_FLAG(flags, IVirtualFs::fsDeferredDeletion))
			{
				container->DeferredDelete();
				stopSearch = true;
			}

			zipFile->Close();
			zipFile->Release();
		}

		err = unzGoToNextFile(uf);
		if (err != UNZ_OK) break;
	}

	return S_OK;
}
