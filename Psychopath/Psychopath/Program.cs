using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace Psychopath
{
    class Psychopath
    {
        enum InjectError
        {
            INJECT_NOT_LOADED = -1,
            INJECT_SUCCESS = 0,
            INJECT_READ_ERROR = 1,
            INJECT_PROCESS_HOLLOWER_INITIALIZATION_FAILED = 2,
            INJECT_PROCESS_CREATE_ERROR = 3,
        }

        [DllImport("psych0p4th.dll", CallingConvention = CallingConvention.Cdecl)]
        static extern int LoadPsych0p4th();

        [DllImport("psych0p4th.dll", CallingConvention = CallingConvention.Cdecl)]
        static extern InjectError ProcessReplaceInject(string inputExe, string targetExe);


        static void Main(string[] args)
        {
            Console.WriteLine("Maniac - < /> Nickel-Hydrogen-Aluminum code");
            Console.WriteLine("Loading mani4c library...");
            if (args.Length == 0)
                ShowUsageAndExit();
            if (LoadPsych0p4th() != 0)
            {
                Console.WriteLine("An error occurred initializing mani4c.");
            }
            string exeToInject = args[0];
            string targetExe = args[1];
            Console.WriteLine("Attempting Process Replacement...");
            InjectError rval = ProcessReplaceInject(exeToInject, targetExe);
            if (rval == 0)
            {
                Console.WriteLine("Successfully injected {0} into process {1}.", exeToInject, targetExe);
            }
            else
            {
                Console.WriteLine("An error was encountered injecting {0} into process {1}.", exeToInject, targetExe);
                Console.WriteLine("Inject returned {0}", Enum.GetName(typeof(InjectError), rval));
            }
        }
        private static void ShowUsageAndExit()
        {
            Console.WriteLine("Error: no target files specified.");
            Console.WriteLine("Usage:\nManiac <exe to inject> <target exe>");
            Environment.Exit(-1);
        }
    }
}
