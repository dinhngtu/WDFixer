#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <cstdlib>
#include "detours.h"

static HMODULE _wininet = nullptr;
static decltype(InternetGetConnectedState)* _realInternetGetConnectedState = nullptr;

class QByteArray;

using qputenv_t = bool (*__cdecl)(char const*, QByteArray*);
static qputenv_t _qputenv = nullptr;

using QByteArray_new_t = void (*__fastcall)(QByteArray*, void*, char const*, int);
static QByteArray_new_t _QByteArray_new = nullptr;

using QByteArray_delete_t = void (*__fastcall)(QByteArray*, void*);
static QByteArray_delete_t _QByteArray_delete = nullptr;

#define QTWEBENGINE_CHROMIUM_FLAGS "QTWEBENGINE_CHROMIUM_FLAGS"

static bool __cdecl MyQputenv(char const* varName, QByteArray* value) {
    if (_qputenv == nullptr)
        return false;
    if (strcmp(varName, QTWEBENGINE_CHROMIUM_FLAGS))
        return _qputenv(varName, value);
    if (_QByteArray_new == nullptr || _QByteArray_delete == nullptr)
        return false;
    auto newstr = static_cast<QByteArray*>(malloc(64));
    _QByteArray_new(newstr, nullptr, "--disable-gpu --no-sandbox", -1);
    auto ret = _qputenv(QTWEBENGINE_CHROMIUM_FLAGS, newstr);
    _QByteArray_delete(newstr, nullptr);
    return ret;
}

static BOOL Attach() {
    _wininet = LoadLibraryW(L"C:\\Windows\\system32\\wininet.dll");
    if (_wininet == nullptr)
        return FALSE;
    _realInternetGetConnectedState = reinterpret_cast<decltype(InternetGetConnectedState)*>(GetProcAddress(_wininet, "InternetGetConnectedState"));
    if (_realInternetGetConnectedState == nullptr)
        return FALSE;

    _qputenv = static_cast<qputenv_t>(DetourFindFunction("Qt5Core.dll", "?qputenv@@YA_NPBDABVQByteArray@@@Z"));
    if (_qputenv == nullptr)
        return FALSE;

    _QByteArray_new = static_cast<QByteArray_new_t>(DetourFindFunction("Qt5Core.dll", "??0QByteArray@@QAE@PBDH@Z"));
    if (_QByteArray_new == nullptr)
        return FALSE;

    _QByteArray_delete = static_cast<QByteArray_delete_t>(DetourFindFunction("Qt5Core.dll", "??1QByteArray@@QAE@XZ"));
    if (_QByteArray_delete == nullptr)
        return FALSE;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&_qputenv, MyQputenv);
    DetourTransactionCommit();

    return TRUE;
}

static BOOL Detach() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&_qputenv, MyQputenv);
    DetourTransactionCommit();

    return TRUE;
}

#if !defined(_WIN64)
#pragma comment(linker, "/EXPORT:InternetGetConnectedState=_InternetGetConnectedState@8")
#endif
EXTERN_C __declspec(dllexport) _Success_(return != FALSE) BOOL STDAPICALLTYPE InternetGetConnectedState(
    _Out_  LPDWORD  lpdwFlags,
    _Reserved_ DWORD dwReserved) {
    if (_realInternetGetConnectedState) {
        return _realInternetGetConnectedState(lpdwFlags, dwReserved);
    }
    else {
        *lpdwFlags = INTERNET_CONNECTION_OFFLINE;
        return FALSE;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        return TRUE;
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Attach, 0, 0, 0);
        return TRUE;
    case DLL_PROCESS_DETACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Detach, 0, 0, 0);
        return TRUE;
    }
}
