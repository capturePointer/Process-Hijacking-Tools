// psych0p4th.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "hollow.h"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

EXTERN_DLL_EXPORT int LoadPsych0p4th()
{
	std::cout << "psych0p4th library initialized!" << std::endl;
	return 0;
}

EXTERN_DLL_EXPORT int ProcessReplaceInject(char* input, char* target)
{
	return x_inJx_procZss(input, target);
}
