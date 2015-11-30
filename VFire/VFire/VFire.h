// VFire.h

#pragma once
#include "psych0p4th.h"
#include "mani4c.h"

#pragma managed
namespace VFire {
	using namespace System;
	using namespace System::Runtime::InteropServices;
	public ref class VFireTools
	{
	public:static
		int VFireTools::InjectDLLIntoProcess(System::String^ dllPath, int targetPid)
	{
		const char* _u_dllPath = (const char*)(Marshal::StringToHGlobalAnsi(dllPath)).ToPointer();
		return _u_InjectDLLIntoProcess(_u_dllPath, targetPid);
	}
	public:static
		int VFireTools::InjectHollowedProcess(System::String^ inputExcecutable, System::String^ hostExcecutable)
	{
		const char* _u_inputExcecutable = (const char*)(Marshal::StringToHGlobalAnsi(inputExcecutable)).ToPointer();
		const char* _u_hostExcecutable = (const char*)(Marshal::StringToHGlobalAnsi(hostExcecutable)).ToPointer();
		return _u_InjectHollowedProcess(_u_inputExcecutable, _u_hostExcecutable);
	}
		   static int _u_InjectDLLIntoProcess(const char* dllPath, int targetPid)
		   {
			   return InjectDll(dllPath, targetPid);
		   }
		   static int _u_InjectHollowedProcess(const char* inputExcecutable, const char* hostExcecutable)
		   {
			   return ProcessReplaceInject((char*)inputExcecutable, (char*)hostExcecutable);
		   }
	};
};