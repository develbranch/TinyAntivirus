#pragma once
#include <TinyAvCore.h>
#include <ioapi.h>

voidpf ZCALLBACK UHOpen  (voidpf opaque, const char* filename, int mode);
voidpf ZCALLBACK UHOpen64(voidpf opaque, const void* filename, int mode);
uLong  ZCALLBACK UHRead  (voidpf opaque, voidpf stream, void* buf, uLong size);
uLong  ZCALLBACK UHWrite (voidpf opaque, voidpf stream, const void* buf, uLong size);
long   ZCALLBACK UHTell  (voidpf opaque, voidpf stream );
ZPOS64_T ZCALLBACK UHTell64(voidpf opaque, voidpf stream );
long   ZCALLBACK UHSeek  (voidpf opaque, voidpf stream, uLong offset, int origin);
long   ZCALLBACK UHSeek64(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin);
int    ZCALLBACK UHClose (voidpf opaque, voidpf stream);
int    ZCALLBACK UHError (voidpf opaque, voidpf stream);
void FillFunctions64( __in voidpf opaque, __out zlib_filefunc64_def* pzlib_filefunc_def);
void FillFunctions( __in voidpf opaque, __out zlib_filefunc_def* pzlib_filefunc_def);