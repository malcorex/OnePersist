gcc ..\source\POC-Injector.c -o POC-Injector.exe -lshell32 -lshlwapi -municode
gcc -shared ..\source\POC-Dll.c -o POC-Dll.dll 