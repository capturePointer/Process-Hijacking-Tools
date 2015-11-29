// mani4c.h

#include "stdafx.h"
#include "injector.h"

#pragma once

using namespace System;

namespace mani4c {

	public ref class Mani4cManaged
	{
		// TODO: Add your methods for this class here.
	};
}
#pragma unmanaged
bool loaded = false;
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

#pragma unmanaged
EXTERN_DLL_EXPORT int LoadMani4c()
{
	std::cout << "mani4c library initialized!" << std::endl;
	loaded = true;
	return 0;
}

#pragma unmanaged
EXTERN_DLL_EXPORT int InjectDll(const char* dllPath, int targetPid)
{
	if (!loaded)
		return -1;
	return injectDllFromPathAndPid(dllPath, targetPid);
}