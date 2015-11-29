#pragma once
#include "stdafx.h"

enum injectError
{
	INJECT_SUCCESS = 0,
	INJECT_ERROR_PROCESS_NOT_FOUND = 1,
	INJECT_ERROR_LOADLIBRARY_NOT_FOUND_IN_KERNEL32 = 2,
	INJECT_ERROR_MEMORY_COULD_NOT_BE_ALLOCATED = 3,
	INJECT_ERROR_COULD_NOT_WRITE_TO_PROCESS_MEMORY = 4,
	INJECT_ERROR_CREATEREMOTETHREAD_FAILED = 5,	
};

int injectDllFromPathAndPid(const char* dllPath, int targetPid) {
	char* buffer = (char*)dllPath;

	/*
	* Get process handle passing in the process ID.
	*/
	int procID = targetPid;
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
	if (process == NULL) {
		return INJECT_ERROR_PROCESS_NOT_FOUND;
	}

	/*
	* Get address of the LoadLibrary function.
	*/
	LPVOID addr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
	if (addr == NULL) {
		//printf("Error: the LoadLibraryA function was not found inside kernel32.dll library.\n");
		return INJECT_ERROR_LOADLIBRARY_NOT_FOUND_IN_KERNEL32;
	}

	/*
	* Allocate new memory region inside the process's address space.
	*/
	LPVOID arg = (LPVOID)VirtualAllocEx(process, NULL, strlen(buffer), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (arg == NULL) {
		return INJECT_ERROR_MEMORY_COULD_NOT_BE_ALLOCATED;
	}

	/*
	* Write the argument to LoadLibraryA to the process's newly allocated memory region.
	*/
	int n = WriteProcessMemory(process, arg, buffer, strlen(buffer), NULL);
	if (n == 0) {
		return INJECT_ERROR_COULD_NOT_WRITE_TO_PROCESS_MEMORY;
	}

	/*
	* Inject our DLL into the process's address space.
	*/
	HANDLE threadID = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, NULL, NULL);
	if (threadID == NULL) {
		return INJECT_ERROR_CREATEREMOTETHREAD_FAILED;
	}
	else {
		//Success!
	}

	/*
	* Close the handle to the process, becuase we've already injected the DLL.
	*/
	CloseHandle(process);
	return INJECT_SUCCESS;
}