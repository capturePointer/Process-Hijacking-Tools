#pragma once
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
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

#pragma unmanaged
EXTERN_DLL_EXPORT int LoadMani4c()
{
	std::cout << "mani4c library initialized!" << std::endl;
	return 0;
}

#pragma unmanaged
EXTERN_DLL_EXPORT int InjectDll(const char* dllPath, int targetPid)
{
	return injectDllFromPathAndPid(dllPath, targetPid);
}