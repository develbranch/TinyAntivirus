#include <TinyAvCore.h>
#include "../FileFs.h"
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <zlib/unzip.h>
#ifdef __cplusplus
}
#endif // __cplusplus

#define  WRITEBUFFERSIZE  ( 16 * 1024)
class CZipFs : public CFileFs
{
protected:
	unz64_file_pos m_currentFilePos;
	virtual ~CZipFs();
public:
	CZipFs();

	virtual HRESULT WINAPI Create(__in LPCWSTR path, __in ULONG const flags) override;

	virtual HRESULT WINAPI Close(void) override;

	virtual HRESULT WINAPI ReCreate(__in_opt void* handle, __in_opt ULONG const flags /*= 0*/) override;
};