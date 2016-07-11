#pragma once
#include "../TinyAvBase.h"

MIDL_INTERFACE("AC50DA10-6F4D-469E-9050-E716FB987AE0")
IFsStream : public IUnknown
{
public:
	enum FsStreamSeek
	{
		FsStreamBegin = 1,	  // Beginning of stream
		FsStreamCurrent = 2,  // Current position of the stream pointer
		FsStreamEnd = 3,	  // End of stream
	};

	BEGIN_INTERFACE

	/* Set handle to the specified stream
	@handle: A handle of new specified stream.
	@return: If the function has no return value.
	*/
	virtual void WINAPI SetFileHandle(__in void* const handle) = 0;

	/* Read data from the specified stream.
	@buffer: A pointer to the buffer that receives the data.
	@bufferSize: The maximum number of bytes to be read.
	@readSize: A pointer to the variable that receives the number of bytes read.
	@return: If the function succeeds, the return value is S_OK.
	*/
	virtual HRESULT WINAPI Read(__out_bcount(bufferSize) LPVOID buffer, __in ULONG bufferSize, __out_opt ULONG * readSize) = 0;

	/* Read a specified number of bytes starting at a specified offset from the specified stream.
	@offset: The number of bytes to move the file pointer. A positive
	value moves the pointer forward in the file and a negative value moves the
	file pointer backward.
	@moveMethod: The starting point for the file pointer move. This parameter
	can be one of the following values:
	FsStreamBegin: The starting point is the beginning of the stream.
	FsStreamCurrent: The start point is the current value of the stream pointer.
	FsStreamEnd: The starting point is the current end-of-stream position.
	@buffer: A pointer to the buffer that receives the data.
	@bufferSize: The maximum number of bytes to be read.
	@readSize: A pointer to the variable that receives the number of bytes read.
	@return: If the function succeeds, the return value is S_OK.
	*/
	virtual HRESULT WINAPI ReadAt(__in LARGE_INTEGER const offset, __in const FsStreamSeek moveMethod, __out_bcount(bufferSize) LPVOID buffer, __in ULONG bufferSize, __out_opt ULONG * readSize) = 0;

	/* Write data to the specified stream.
	@buffer: A pointer to the buffer containing the data to be written to stream.
	@bufferSize: The maximum number of bytes to be written.
	@writtenSize: A pointer to the variable that receives the number of bytes written.
	@return: If the function succeeds, the return value is S_OK.
	*/
	virtual HRESULT WINAPI Write(__in_bcount(bufferSize) LPCVOID buffer, __in ULONG bufferSize, __out_opt ULONG * writtenSize) = 0;

	/* Write a specified number of bytes starting at a specified offset from the specified stream.
	@offset: The number of bytes to move the file pointer. A positive
	value moves the pointer forward in the file and a negative value moves the
	file pointer backward.
	@moveMethod: The starting point for the file pointer move. This parameter
	can be one of the following values:
		FsStreamBegin: The starting point is the beginning of the stream.
		FsStreamCurrent: The start point is the current value of the stream pointer.
		FsStreamEnd: The starting point is the current end-of-stream position.
	@buffer: A pointer to the buffer that receives the data.
	@bufferSize: The maximum number of bytes to be read.
	@readSize: A pointer to the variable that receives the number of bytes read.
	@return: If the function succeeds, the return value is S_OK.
	*/
	virtual HRESULT WINAPI WriteAt(__in LARGE_INTEGER const offset, __in const FsStreamSeek moveMethod, __in_bcount(bufferSize) LPCVOID buffer, __in ULONG bufferSize, __out_opt ULONG * writtenSize) = 0;

	// Get the stream pointer of the specified stream.
	//@pos: A pointer to a variable to receive the new file pointer.
	//@return: If the function succeeds, the return value is S_OK.
	virtual HRESULT WINAPI Tell(__out ULARGE_INTEGER * pos) = 0;

	/* Move the stream pointer of the specified stream.
	@distanceToMove: The number of bytes to move the file pointer. A positive
	value moves the pointer forward in the file and a negative value moves the
	file pointer backward.
	@pos: A pointer to a variable to receive the new file pointer. If this 
	parameter is NULL, the new file pointer is not returned.
	@MoveMethod: The starting point for the file pointer move. This parameter
	can be one of the following values:
		FsStreamBegin: The starting point is the beginning of the stream.
		FsStreamCurrent: The start point is the current value of the stream pointer.
		FsStreamEnd: The starting point is the current end-of-stream position.
	@return: If the function succeeds, the return value is S_OK.
	*/
	virtual HRESULT WINAPI Seek(__out_opt ULARGE_INTEGER * pos, __in LARGE_INTEGER const distanceToMove, __in const FsStreamSeek MoveMethod) = 0;

	/*
	Shrink the size of a stream to  the current position of the stream pointer.
	@return: If the function succeeds, the return value is S_OK.
	*/
	virtual HRESULT WINAPI Shrink(void) = 0;

	END_INTERFACE
};
