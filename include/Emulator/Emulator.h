#pragma once
#include "../TinyAvBase.h"
#include "../FileType/PEFile.h"

MIDL_INTERFACE("D5B16D11-F05D-4F5A-90CD-849EC0AE4049")
IEmulObserver: public IUnknown
{
public:

	enum EmulErrorCode
	{
		EmulatorErr = EMULATOR_ERROR_CODE_BASE,
		EmulatorIsNotFound,
		EmulatorIsNotRunable,
		EmulatorInternalError
	};

	BEGIN_INTERFACE

	// Method is called when emulator starts
	virtual HRESULT WINAPI OnEmulatorStarting(void) = 0;

	// Method is called when emulator stops
	virtual HRESULT WINAPI OnEmulatorStopped(void) = 0;

	// method is called when an error occurred.
	virtual void WINAPI OnError(__in DWORD const dwErrorCode) = 0;
	END_INTERFACE
};

// IEmulator interface
MIDL_INTERFACE("D6C8BC2A-2F66-4DEC-9E63-8A4DF52BE7DF")
IEmulator: public IUnknown
{
public:
	enum EmulateOrigin
	{
		FromImageBase = 1,
		FromEntryPoint,
	};

	BEGIN_INTERFACE

	/* Read register value
	
	@reg: register ID that is to be retrieved.
	@regValue: pointer to a variable storing the register value.

	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI ReadRegister(__in DWORD const reg, __out DWORD_PTR *regValue) = 0;

	/* Write register value
	
	@reg: register ID that is to be retrieved.
	@regValue: variable storing the register value to write.

	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI WriteRegister(__in DWORD const reg, __out DWORD_PTR const regValue) = 0;

	/*
	Read a range of bytes in memory.

	@memoryAddr: starting memory address of bytes to get.
	@lpBuffer:   pointer to a variable containing data copied from memory.
	@nNumberOfBytesToRead:   size of memory to read.

	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI ReadMemory(__in DWORD_PTR memoryAddr, __out_bcount(nNumberOfBytesToRead) LPVOID lpBuffer, __in DWORD nNumberOfBytesToRead) = 0;

	/*
	Write to a range of bytes in memory.

	@memoryAddr: starting memory address of bytes to set.
	@lpBuffer:   pointer to a variable containing data to be written to memory.
	@nNumberOfBytesToWrite:   size of memory to write to.

	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI WriteMemory(__in DWORD_PTR memoryAddr, __out_bcount(nNumberOfBytesToWrite) LPVOID lpBuffer, __in DWORD nNumberOfBytesToWrite) = 0;

	/*
	Stop emulation 
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI StopEmulator(void) = 0;

	/*
	Emulate machine code.

	@lpCodeBuffer: pointer to a variable containing data to be emulated.
	@nSizeOfCode: size of memory to emulate.
	@memoryMappedAddr: address where machine code is mapped.
	@nSizeOfStackCommit: size of the stack in bytes.
	@nSizeOfStackReserve: total stack allocation in mapped memory.
	@addressToStart: address where emulation starts.
	@nNumberOfBytesToEmulate: the number of instructions to be emulated. When this value is 0,
	we will emulate all the code available, until the code is finished.

	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI EmulateCode(__in_bcount(nSizeOfCode) LPBYTE lpCodeBuffer, __in DWORD nSizeOfCode,
			__in DWORD_PTR memoryMappedAddr, __in DWORD nSizeOfStackCommit, __in DWORD nSizeOfStackReserve,
			__in DWORD_PTR addressToStart, __in DWORD nNumberOfBytesToEmulate) = 0;

	/*
	Emulate PE file.

	@peFile: pointer to IPeFile object.
	@rvaToStart: address where emulation starts.
	@origin: Position used as reference for the rvaToStart. It is specified by one of the following constants
		FromImageBase: Beginning of image
		FromEntryPoint: from Address of entry point
	@nNumberOfBytesToEmulate: the number of instructions to be emulated. When this value is 0,
	we will emulate all the code available, until the code is finished.
	
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI EmulatePeFile(__in IPeFile *peFile, __in DWORD_PTR rvaToStart, __in int origin, __in DWORD nNumberOfBytesToEmulate = 0) = 0;

	//virtual HRESULT WINAPI EmulatePe64File(__in IPe64File *peFile, __in DWORD_PTR rvaToStart, __in int origin, __in DWORD nNumberOfBytesToEmulate = 0) = 0;

	/*
	Add an emulator Observer
	@observer: a pointer to IEmulObserver object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI AddObserver(__in IEmulObserver *observer) = 0;

	/*
	Remove an emulator Observer
	@observer: a pointer to IEmulObserver object
	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI RemoveObserver(__in IEmulObserver *observer) = 0;

	/*
	Register callback for a hook event.
	The callback will be run when the hook event is hit.

	@hookHandle: hook handle returned from this registration. To be used in uc_hook_del() API
	@type: hook type
	@callback: callback to be run when instruction is hit
	@user_data: user-defined data. This will be passed to callback function in its
	last argument @user_data
	@...: variable arguments (depending on @type)

	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT __cdecl AddHook(__out void *hookHandle, __in int type, __in void * callback, __in void *user_data, ...) = 0;

	/*
	Unregister (remove) a hook callback.
	This API removes the hook callback registered by AddHook()
	
	@hookHandle: handle returned by uc_hook_add()

	@return: HRESULT on success, or other value on failure.
	*/
	virtual HRESULT WINAPI RemoveHook(__in size_t hookHandle) = 0;

	END_INTERFACE
};