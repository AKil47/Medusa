#include <windows.h>

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
}
