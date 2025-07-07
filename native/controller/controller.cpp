#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>

HWND qt_creator_window = NULL;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    char window_title[256];
    GetWindowTextA(hwnd, window_title, sizeof(window_title));

    if (strstr(window_title, "Qt Creator") != NULL) {
        qt_creator_window = hwnd;
        return FALSE;
    }
    return TRUE;
}

DWORD FindQtCreatorProcess() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &pe32)) {
        do {
            std::string processName = pe32.szExeFile;
            if (processName.find("qtcreator") != std::string::npos || 
                processName.find("Qt Creator") != std::string::npos) {
                CloseHandle(snapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(snapshot, &pe32));
    }

    CloseHandle(snapshot);
    return 0;
}

extern "C" {
    __declspec(dllexport) int test_controller() {
        return 42;
    }

    __declspec(dllexport) int run_qt_creator() {
        qt_creator_window = NULL;
        EnumWindows(EnumWindowsProc, 0);

        if (!qt_creator_window) {
            return 0;
        }

        SetForegroundWindow(qt_creator_window);
        Sleep(100);

        INPUT inputs[4] = {};

        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_CONTROL;
        inputs[0].ki.dwFlags = 0;

        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = 'R';
        inputs[1].ki.dwFlags = 0;

        inputs[2].type = INPUT_KEYBOARD;
        inputs[2].ki.wVk = 'R';
        inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

        inputs[3].type = INPUT_KEYBOARD;
        inputs[3].ki.wVk = VK_CONTROL;
        inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(4, inputs, sizeof(INPUT));
        Sleep(100);

        ShowWindow(qt_creator_window, SW_MINIMIZE);

        return 1;
    }

    __declspec(dllexport) int inject_hook_into_qtcreator() {
        DWORD processId = FindQtCreatorProcess();
        if (processId == 0) {
            return 0; // Qt Creator process not found
        }

        // Get the path to the hook DLL
        char dllPath[MAX_PATH];
        HMODULE hModule = GetModuleHandleA("medusa_controller.dll");
        if (hModule == NULL) {
            return -1; // Failed to get module handle
        }

        GetModuleFileNameA(hModule, dllPath, MAX_PATH);
        
        // Replace controller DLL name with hook DLL name
        std::string dllPathStr = dllPath;
        size_t pos = dllPathStr.find("medusa_controller.dll");
        if (pos != std::string::npos) {
            dllPathStr.replace(pos, strlen("medusa_controller.dll"), "medusa_hook.dll");
        } else {
            return -2; // Failed to construct hook DLL path
        }

        // Open the target process
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        if (hProcess == NULL) {
            return -3; // Failed to open process
        }

        // Allocate memory in the target process for the DLL path
        LPVOID remoteMemory = VirtualAllocEx(hProcess, NULL, dllPathStr.length() + 1, 
                                           MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (remoteMemory == NULL) {
            CloseHandle(hProcess);
            return -4; // Failed to allocate memory
        }

        // Write the DLL path to the target process
        if (!WriteProcessMemory(hProcess, remoteMemory, dllPathStr.c_str(), 
                               dllPathStr.length() + 1, NULL)) {
            VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return -5; // Failed to write memory
        }

        // Get the address of LoadLibraryA
        HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
        if (hKernel32 == NULL) {
            VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return -6; // Failed to get kernel32 handle
        }

        LPVOID loadLibraryAddr = GetProcAddress(hKernel32, "LoadLibraryA");
        if (loadLibraryAddr == NULL) {
            VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return -7; // Failed to get LoadLibraryA address
        }

        // Create a remote thread to load the DLL
        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, 
                                          (LPTHREAD_START_ROUTINE)loadLibraryAddr, 
                                          remoteMemory, 0, NULL);
        if (hThread == NULL) {
            VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return -8; // Failed to create remote thread
        }

        // Wait for the thread to complete
        WaitForSingleObject(hThread, INFINITE);

        // Get the thread exit code (handle to the loaded DLL)
        DWORD exitCode;
        GetExitCodeThread(hThread, &exitCode);

        // Clean up
        CloseHandle(hThread);
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        return exitCode != 0 ? 1 : -9; // Return 1 if DLL was loaded successfully
    }
}
