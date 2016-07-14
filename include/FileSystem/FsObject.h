#pragma once
#include "../TinyAvBase.h"
#include "FsAttribute.h"
#include "FsStream.h"

MIDL_INTERFACE("9FC5CDB9-77CF-4D3D-802A-74BBF789F572")
IVirtualFs : public IUnknown
{
    enum IFsType {
    unknown = 1,
    basic   = 1 << 1,
    archive = 1 << 2,
    };

    enum IFsObjectFlags {
        fsRead              = 1,        // Read access
        fsWrite             = 1 << 1,   // Write access
        fsSharedRead        = 1 << 2,   // Enables subsequent open operations on a file to request read access.
        fsSharedWrite       = 1 << 3,   // Enables subsequent open operations on a file to request write access.
        fsSharedDelete      = 1 << 4,   // Enables subsequent open operations on a file to request delete access.
        fsCreateNew         = 1 << 5,   // Creates a new file, only if it does not already exist.
        fsCreateAlways      = 1 << 6,   // Creates a new file, always.
        fsOpenAlways        = 1 << 7,   // Opens a file, always.
        fsOpenExisting      = 1 << 8,   // Opens a file only if it exists.
        fsAttrNormal        = 1 << 9,   // The file does not have other attributes set. 
        fsAttrReadonly      = 1 << 10,  // The file is read only. Applications can read the file, but cannot write to or delete it.
        fsAttrSystem        = 1 << 11,  // The file is part of or used exclusively by an operating system.
        fsAttrHidden        = 1 << 12,  // The file is hidden. Do not include it in an ordinary directory listing.
        fsAttrTemporary     = 1 << 13,  // The file is being used for temporary storage.
        fsAttrDeleteOnClose = 1 << 14,  // The file is to be deleted immediately after all of its handles are closed.
        fsDeferredCreation  = 1 << 15,  // Defer the creation of file when creating or opening until application re-creates file.
        fsDeferredDeletion  = 1 << 16,  // Defer the deletion of file until application closes it
    };

    BEGIN_INTERFACE

public:
    /* Create or open file
    @lpFileName: The name of the file to be created or opened.
    @flags:    The file attributes and flags.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI Create(__in LPCWSTR lpFileName, __in ULONG const flags) = 0;

    /* Close file
    @return: HRESULT on success, or other value on failure.
    */

    virtual HRESULT WINAPI Close(void) = 0;
    /* Close and re-create or re-open file OR set file handle to existing handle and new flags.
    @handle: new file handle
    @flags:    The file attributes and flags.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI ReCreate(__in_opt void* handle = NULL, __in_opt ULONG const flags = 0) = 0;

    /* Get current opened file handle
    @fileHandle: a pointer to a variable storing file handle.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI GetHandle(__out LPVOID * fileHandle) = 0;

    /* Get object type
    @fsType: a pointer to a variable storing type. Type is one of these values:
        basic: basic file system
        archive: archive file system
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI GetFsType(__out ULONG * fsType) = 0;

    /* Check whether file was already opened
    @isOpened: a pointer to a variable storing result.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI IsOpened(__out BOOL *isOpened) = 0;

    /* Retrieve the last-error code value.
    @return: return value is the last-error code.
    */
    virtual ULONG WINAPI GetError(void) = 0;

    /* Sets the last-error code.
    @return: This function does not return a value.
    */
    virtual void WINAPI SetError(__in const ULONG error) = 0;

    /* Retrieve the flags of file object.
    @flags: a pointer to a variable storing result.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI GetFlags(__out ULONG *flags) = 0;

    /* Retrieve fully qualified name of file.
    @fullPath: a pointer to a pointer to store file name.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI GetFullPath(__out BSTR *fullPath) = 0;

    /* Retrieve file name
    @fullPath: a pointer to a pointer to store file name.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI GetFileName(__out BSTR *fileName) = 0;

    /* Retrieve file extension
    @fullPath: a pointer to a pointer to store extension.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI GetFileExt(__out BSTR *fileExt) = 0;

    /* Retrieve container object
    @container: a pointer to a pointer to store container object.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI GetContainer(__out IVirtualFs **container) = 0;

    /* Set container object
    @container: a pointer to a container object.
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI SetContainer(__in IVirtualFs *container) = 0;

    /* Delete current file
    @return: HRESULT on success, or other value on failure.
    */
    virtual HRESULT WINAPI DeferredDelete(void) = 0;

    END_INTERFACE
};

