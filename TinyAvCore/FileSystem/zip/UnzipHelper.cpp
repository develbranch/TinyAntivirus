#include "UnzipHelper.h"

#ifdef _DEBUG
#pragma comment(lib, "zlibstaticd.lib")
 #else
#pragma comment(lib, "zlibstatic.lib")
 #endif // _DEBUG

static void TranslateOpenMode(__in int mode, __out ULONG *creationMode)
{
	*creationMode = 0;

	if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ)
	{
		*creationMode = IVirtualFs::fsRead | IVirtualFs::fsOpenExisting | IVirtualFs::fsSharedRead | IVirtualFs::fsAttrNormal;
	}
	else if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
	{
		*creationMode = IVirtualFs::fsRead | IVirtualFs::fsWrite | IVirtualFs::fsSharedRead | IVirtualFs::fsOpenExisting | IVirtualFs::fsAttrNormal;
	}
	else if (mode & ZLIB_FILEFUNC_MODE_CREATE)
	{
		*creationMode = IVirtualFs::fsRead | IVirtualFs::fsWrite | IVirtualFs::fsSharedRead | IVirtualFs::fsCreateAlways | IVirtualFs::fsAttrNormal;
	}
}

voidpf ZCALLBACK UHOpen(voidpf opaque, const char* filename, int mode)
{
	return UHOpen64(opaque, (const void*)filename, mode);
}

uLong ZCALLBACK UHRead(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	UNREFERENCED_PARAMETER(opaque);
	uLong  readSize = 0;

	IFsStream* fileStream = static_cast<IFsStream*>(stream);
	if (fileStream == NULL) return 0;

	if (!SUCCEEDED(fileStream->Read(buf, size, &readSize)))
		readSize = 0;

	return readSize;
}

uLong ZCALLBACK UHWrite(voidpf opaque, voidpf stream, const void* buf, uLong size)
{
	UNREFERENCED_PARAMETER(opaque);
	uLong  writtenSize = 0;
	IFsStream* fileStream = static_cast<IFsStream*>(stream);
	if (fileStream == NULL) return 0;
	if (!SUCCEEDED(fileStream->Write(buf, size, &writtenSize)))
		writtenSize = 0;
	return writtenSize;
}

long ZCALLBACK UHTell(voidpf opaque, voidpf stream)
{
	return (long)UHTell64(opaque, stream);
}

long ZCALLBACK UHSeek(voidpf opaque, voidpf stream, uLong offset, int origin)
{
	return UHSeek64(opaque, stream, (ZPOS64_T)offset, origin);
}

int ZCALLBACK UHClose(voidpf opaque, voidpf stream)
{
	UNREFERENCED_PARAMETER(opaque);
	int ret = -1;
	IVirtualFs * file = static_cast<IVirtualFs*>(opaque);
	if (file == NULL) return 0;
	ret = (SUCCEEDED(file->Close())) ? 0 : -1;
	IFsStream* fileStream = static_cast<IFsStream*>(stream);
	if (fileStream) fileStream->Release();
	file->Release();
	return ret;
}

int ZCALLBACK UHError(voidpf opaque, voidpf stream)
{
	UNREFERENCED_PARAMETER(stream);
	IVirtualFs * file = static_cast<IVirtualFs*>(opaque);
	if (file == NULL) return -1;
	return (int)file->GetError();
}

voidpf ZCALLBACK UHOpen64(voidpf opaque, const void* filename, int mode)
{
	UNREFERENCED_PARAMETER(filename);
	ULONG creationMode = IVirtualFs::fsRead | IVirtualFs::fsSharedRead | IVirtualFs::fsOpenExisting | IVirtualFs::fsAttrNormal;
	voidpf zipHandle = NULL;
	TranslateOpenMode(mode, &creationMode);

	IVirtualFs * file = static_cast<IVirtualFs*>(opaque);
	if (file == NULL) return zipHandle;

	ULONG fsType = 0;

	HRESULT hr = E_FAIL;
	if (SUCCEEDED(file->GetFsType(&fsType)) && fsType == IVirtualFs::archive)
	{
		void* handle;
		hr = file->GetHandle((LPVOID*)&handle);
		if (SUCCEEDED(hr))
			hr = file->ReCreate(handle, creationMode);
	}
	else
	{
		hr = file->ReCreate(NULL, creationMode);
	}

	if (SUCCEEDED(hr))
	{
		IFsStream* fileStream = NULL;
		if (SUCCEEDED(file->QueryInterface(__uuidof(IFsStream), (LPVOID*)&fileStream)))
		{
			file->AddRef();
			zipHandle = (voidpf)fileStream;
		}
	}

	return zipHandle;
}

ZPOS64_T ZCALLBACK UHTell64(voidpf opaque, voidpf stream)
{
	UNREFERENCED_PARAMETER(opaque);
	ULARGE_INTEGER pos = {};
	IFsStream* fileStream = static_cast<IFsStream*>(stream);
	if (fileStream == NULL) return 0;
	if (SUCCEEDED(fileStream->Tell(&pos)))
		return pos.QuadPart;
	return 0;
}

long ZCALLBACK UHSeek64(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
{
	UNREFERENCED_PARAMETER(opaque);
	ULARGE_INTEGER pos = {};
	LARGE_INTEGER distanceToMove = {};
	long ret = -1;

	IFsStream::FsStreamSeek dwMoveMethod;

	switch (origin)
	{
	case ZLIB_FILEFUNC_SEEK_CUR:
		dwMoveMethod = IFsStream::FsStreamCurrent;
		break;
	case ZLIB_FILEFUNC_SEEK_END:
		dwMoveMethod = IFsStream::FsStreamEnd;
		break;
	case ZLIB_FILEFUNC_SEEK_SET:
		dwMoveMethod = IFsStream::FsStreamBegin;
		break;
	default:
		return -1;
	}

	distanceToMove.QuadPart = offset;

	IFsStream* fileStream = static_cast<IFsStream*>(stream);
	if (fileStream == NULL) return 0;
	HRESULT hr = fileStream->Seek(&pos, distanceToMove, dwMoveMethod);
	if (SUCCEEDED(hr))
		ret = 0;
	else
	{
		IVirtualFs * file = static_cast<IVirtualFs*>(stream);
		if (file) file->SetError((ULONG)(hr & 0xffff));
		ret = (long)hr;
	}

	return ret;
}

void FillFunctions64(__in voidpf opaque, __out zlib_filefunc64_def* pzlib_filefunc_def)
{
	pzlib_filefunc_def->zopen64_file = UHOpen64;
	pzlib_filefunc_def->zread_file = UHRead;
	pzlib_filefunc_def->zwrite_file = UHWrite;
	pzlib_filefunc_def->ztell64_file = UHTell64;
	pzlib_filefunc_def->zseek64_file = UHSeek64;
	pzlib_filefunc_def->zclose_file = UHClose;
	pzlib_filefunc_def->zerror_file = UHError;
	pzlib_filefunc_def->opaque = opaque;
}

void FillFunctions(__in voidpf opaque, __out zlib_filefunc_def* pzlib_filefunc_def)
{
	pzlib_filefunc_def->zopen_file = UHOpen;
	pzlib_filefunc_def->zread_file = UHRead;
	pzlib_filefunc_def->zwrite_file = UHWrite;
	pzlib_filefunc_def->ztell_file = UHTell;
	pzlib_filefunc_def->zseek_file = UHSeek;
	pzlib_filefunc_def->zclose_file = UHClose;
	pzlib_filefunc_def->zerror_file = UHError;
	pzlib_filefunc_def->opaque = opaque;
}
