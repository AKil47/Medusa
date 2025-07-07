#include <windows.h>
#include <detours.h>
#include <fstream>
#include <string>
#include <sstream>
#include <mutex>

// Original function pointers
static BOOL (WINAPI *TrueCreateProcessW)(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) = CreateProcessW;

static BOOL (WINAPI *TrueCreateProcessA)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) = CreateProcessA;

// Mutex for thread-safe logging
static std::mutex logMutex;

// Logging function
void LogProcessCreation(const std::string& processName, const std::string& commandLine, DWORD processId) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::ofstream logFile("C:\\Users\\amani\\Desktop\\logs.txt", std::ios::app);
    if (logFile.is_open()) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        
        char timestamp[100];
        sprintf_s(timestamp, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 
                 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        
        logFile << "[" << timestamp << "] Process Created: " << processName 
                << " | PID: " << processId 
                << " | Command: " << commandLine << std::endl;
        logFile.close();
    }
}

// Hook function for CreateProcessW
BOOL WINAPI HookedCreateProcessW(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) {
    // Call the original function
    BOOL result = TrueCreateProcessW(
        lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
        bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
        lpStartupInfo, lpProcessInformation
    );
    
    if (result && lpProcessInformation) {
        // Convert wide strings to regular strings for logging
        std::string appName = "Unknown";
        std::string cmdLine = "Unknown";
        
        if (lpApplicationName) {
            int len = WideCharToMultiByte(CP_UTF8, 0, lpApplicationName, -1, nullptr, 0, nullptr, nullptr);
            if (len > 0) {
                char* buffer = new char[len];
                WideCharToMultiByte(CP_UTF8, 0, lpApplicationName, -1, buffer, len, nullptr, nullptr);
                appName = buffer;
                delete[] buffer;
            }
        }
        
        if (lpCommandLine) {
            int len = WideCharToMultiByte(CP_UTF8, 0, lpCommandLine, -1, nullptr, 0, nullptr, nullptr);
            if (len > 0) {
                char* buffer = new char[len];
                WideCharToMultiByte(CP_UTF8, 0, lpCommandLine, -1, buffer, len, nullptr, nullptr);
                cmdLine = buffer;
                delete[] buffer;
            }
        }
        
        LogProcessCreation(appName, cmdLine, lpProcessInformation->dwProcessId);
    }
    
    return result;
}

// Hook function for CreateProcessA
BOOL WINAPI HookedCreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) {
    // Call the original function
    BOOL result = TrueCreateProcessA(
        lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
        bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
        lpStartupInfo, lpProcessInformation
    );
    
    if (result && lpProcessInformation) {
        std::string appName = lpApplicationName ? lpApplicationName : "Unknown";
        std::string cmdLine = lpCommandLine ? lpCommandLine : "Unknown";
        
        LogProcessCreation(appName, cmdLine, lpProcessInformation->dwProcessId);
    }
    
    return result;
}

// Install hooks
BOOL InstallHooks() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    DetourAttach(&(PVOID&)TrueCreateProcessW, HookedCreateProcessW);
    DetourAttach(&(PVOID&)TrueCreateProcessA, HookedCreateProcessA);
    
    LONG error = DetourTransactionCommit();
    return error == NO_ERROR;
}

// Uninstall hooks
BOOL UninstallHooks() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    DetourDetach(&(PVOID&)TrueCreateProcessW, HookedCreateProcessW);
    DetourDetach(&(PVOID&)TrueCreateProcessA, HookedCreateProcessA);
    
    LONG error = DetourTransactionCommit();
    return error == NO_ERROR;
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Install hooks when DLL is loaded
        if (InstallHooks()) {
            LogProcessCreation("MEDUSA_HOOK", "Hook DLL loaded and hooks installed", GetCurrentProcessId());
        }
        break;
    case DLL_PROCESS_DETACH:
        // Uninstall hooks when DLL is unloaded
        UninstallHooks();
        LogProcessCreation("MEDUSA_HOOK", "Hook DLL unloaded and hooks removed", GetCurrentProcessId());
        break;
    }
    return TRUE;
}

extern "C" {
    __declspec(dllexport) int test_hook() {
        return 24;
    }
}