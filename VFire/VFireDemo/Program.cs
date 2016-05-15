using System;

namespace VFire.Demo
{
    internal class VFireDemo
    {
        private static void Main(string[] args)
        {
            Console.WriteLine(VFireTools.InjectDLLIntoProcess("D:\\AnnoyingBox.dll", 7924));
            Console.WriteLine(VFireTools.InjectHollowedProcess("C:\\Program Files (x86)\\PuTTY\\putty.exe",
                "C:\\Windows\\System32\\calc.exe"));
            Console.ReadKey();
        }
    }
}