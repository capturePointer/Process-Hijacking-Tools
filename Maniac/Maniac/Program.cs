using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Reflection;

namespace Maniac
{
    class ManiacInjectTool
    {
        enum InjectError
        {
            MANI4C_NOT_INITIALIZED = -1,
            INJECT_SUCCESS = 0,
            INJECT_ERROR_PROCESS_NOT_FOUND = 1,
            INJECT_ERROR_LOADLIBRARY_NOT_FOUND_IN_KERNEL32 = 2,
            INJECT_ERROR_MEMORY_COULD_NOT_BE_ALLOCATED = 3,
            INJECT_ERROR_COULD_NOT_WRITE_TO_PROCESS_MEMORY = 4,
            INJECT_ERROR_CREATEREMOTETHREAD_FAILED = 5,
        }

        [DllImport("mani4c.dll", CallingConvention = CallingConvention.Cdecl)]
        static extern int LoadMani4c();

        [DllImport("mani4c.dll", CallingConvention = CallingConvention.Cdecl)]
        static extern InjectError InjectDll(string dllPath, int targetPid);


        static void Main(string[] args)
        {
            Console.WriteLine("Maniac - < /> Nickel-Hydrogen-Aluminum code");
            Console.WriteLine("Loading mani4c library...");
            if (args.Length == 0)
                ShowUsageAndExit();
            if (LoadMani4c()!=0)
            {
                Console.WriteLine("An error occurred initializing mani4c.");
            }
            string dllToInject = args[0];
            int targetPid = int.Parse(args[1]);
            Console.WriteLine("Attempting DLL Injection...");
            InjectError rval = InjectDll(dllToInject, targetPid);
            if (rval == 0)
            {
                Console.WriteLine("Successfully injected {0} into process {1}.", dllToInject, targetPid);
            }
            else
            {
                Console.WriteLine("An error was encountered injecting {0} into process {1}.", dllToInject, targetPid);
                Console.WriteLine("Inject returned {0}", Enum.GetName(typeof(InjectError), rval));
            }
        }

        private static void ShowUsageAndExit()
        {
            Console.WriteLine("Error: no target files specified.");
            Console.WriteLine("Usage:\nManiac <file to inject.dll> <target pid>");
            Environment.Exit(-1);
        }
    }
}
