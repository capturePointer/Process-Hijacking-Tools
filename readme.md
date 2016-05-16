#Random Process Hijacking tools!
These are all proof-of-concept, put together from patched and modified public domain code from around the web and analysis of certain programs.

A collection of tools that replace processes and inject DLLs.
Malware often uses these techniques, so hopefully by creating these tools, threat analysis can become more advanced at recognizing these common techniques.

This project is written in C++/CLI and C#. Full source code and built binaries are both available.


# Status:
- This has been confirmed to work on Windows 10 build 10240 and 10586, and possibly later versions.


#Contents
- These tools contain process hijacking tools.
- They can perform efficient and (mostly) reliable process replacement and DLL injection.
- This tool has been tested on some commonly used programs, such as calc.exe (Windows Calculator) and cmd.exe (Command Prompt)
- The DLL injection tool has a very high success rate.
- The Process Replacement tool starts a process suspended, re-allocates and overwrites its memory, then changes the Entry Point register to the entry point of the new program. The tool then restarts the (replaced) process

### Virus scanners
- Your virus scanner may very well block these; after all these programs are just an exposed version of techniques commonly used by malware. We hope to improve security by making these techniques public, so antivirus can be designed to detect them.
- See [VirusTotal's unsurprising detections for this project's binaries](https://www.virustotal.com/en/file/04b4ff1d197588caa7deba21454dd44880f1d137c6c7a69655caf533b2390c49/analysis/1448843366/).

By using this code and/or the binaries affiliated with this code, you agree to not use this code for any malicious purpose; doing so is strictly prohibited and is against the spirit of this project.
