using System;
using VFire;

namespace PurpleSyringe
{
    internal class Program
    {
        private static void Main(string[] args)
        {
            Console.WriteLine("PurpleSyringe v1.1.0 - < /> Nickel-Hydrogen-Aluminum code");
            if (args.Length != 3 && args.Length != 0)
                ShowUsageAndExit();
            if (args.Length == 0)
            {
                InteractiveSyringe();
                return;
            }
            if (args[0] == "inject")
            {
                VFireTools.InjectDLLIntoProcess(args[1], int.Parse(args[2]));
            }
            else if (args[0] == "replace")
            {
                VFireTools.InjectHollowedProcess(args[1], args[2]);
            }
            else if (args[0] == "interactive")
            {
                InteractiveSyringe();
            }
            else
            {
                ShowUsageAndExit();
            }
        }

        private static void ShowUsageAndExit()
        {
            Console.WriteLine("Usage:");
            Console.WriteLine("PurpleSyringe inject [input DLL] [target PID]");
            Console.WriteLine("PurpleSyringe replace [input EXE] [target EXE]");
            Console.WriteLine("PurpleSyringe interactive");
            Environment.Exit(-1);
        }

        private static void InteractiveSyringe()
        {
            try
            {
                Console.WriteLine("PurpleSyringe - Interactive Mode");
                Console.Write("[inject/replace]>>>");
                var action = Console.ReadLine();
                if (action == "inject")
                {
                    Console.Write("[DLL Path]>>>");
                    var dPath = Console.ReadLine();
                    Console.Write("[PID]>>>");
                    var pid = int.Parse(Console.ReadLine());
                    var s = VFireTools.InjectDLLIntoProcess(dPath, pid);
                    Console.WriteLine("Result: {0}", s);
                }
                if (action == "replace")
                {
                    Console.Write("[Input EXE]>>>");
                    var input = Console.ReadLine();
                    Console.Write("[Host EXE]>>>");
                    var host = Console.ReadLine();
                    var s = VFireTools.InjectHollowedProcess(input, host);
                    Console.WriteLine("Result: {0}", s);
                }
            }
            catch
            {
                Console.WriteLine("An error occurred.");
                Environment.Exit(-1);
            }
        }
    }
}