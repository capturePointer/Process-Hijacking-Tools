using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VFire;

namespace PurpleSyringe
{
    class Program
    {
        static void Main(string[] args)
        {
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
        static void ShowUsageAndExit()
        {
            Console.WriteLine("Usage:");
            Console.WriteLine("PurpleSyringe inject [input DLL] [target PID]");
            Console.WriteLine("PurpleSyringe replace [input EXE] [target EXE]");
            Console.WriteLine("PurpleSyringe interactive");
            Environment.Exit(-1);
        }
        static void InteractiveSyringe()
        {
            try
            {
                Console.WriteLine("PurpleSyringe - Interactive Mode");
                Console.Write("[inject/replace]>>>");
                string action = Console.ReadLine();
                if (action == "inject")
                {
                    Console.Write("[DLL Path]>>>");
                    string dPath = Console.ReadLine();
                    Console.Write("[PID]>>>");
                    int pid = int.Parse(Console.ReadLine());
                    int s = VFireTools.InjectDLLIntoProcess(dPath, pid);
                    Console.WriteLine("Result: {0}", s);
                }
                if (action == "replace")
                {
                    Console.Write("[Input EXE]>>>");
                    string input = Console.ReadLine();
                    Console.Write("[Host EXE]>>>");
                    string host = Console.ReadLine();
                    int s = VFireTools.InjectHollowedProcess(input, host);
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
