/*
 * Author: ahura (malcorex)
 * Github: https://github.com/malcorex
 * Channel: https://t.me/Darkbitx
 * SPDX-License-Identifier: GPL-3.0-only
 */


#include <windows.h>
#include <stdio.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);

            MessageBoxW(NULL, L"Done", L"Info", MB_OK);
            break;
            
        case DLL_PROCESS_DETACH:
            break;
            
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

// iR