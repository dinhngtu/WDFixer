#include "stdafx.h"
#include "qt5core.h"
#include "detours.h"

static qputenv_t _qputenv = nullptr;
static QByteArray_new_t _QByteArray_new = nullptr;
static QByteArray_delete_t _QByteArray_delete = nullptr;
static std::once_flag _qt5core_flag;

#define QTWEBENGINE_CHROMIUM_FLAGS "QTWEBENGINE_CHROMIUM_FLAGS"

static void InitQt5Core() {
    _QByteArray_new = static_cast<QByteArray_new_t>(DetourFindFunction("Qt5Core.dll", "??0QByteArray@@QAE@PBDH@Z"));
    if (_QByteArray_new == nullptr)
        return;

    _QByteArray_delete = static_cast<QByteArray_delete_t>(DetourFindFunction("Qt5Core.dll", "??1QByteArray@@QAE@XZ"));
    if (_QByteArray_delete == nullptr)
        return;
}

static bool __cdecl MyQputenv(char const* varName, QByteArray* value) {
    std::call_once(_qt5core_flag, InitQt5Core);
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
    _qputenv = static_cast<qputenv_t>(DetourFindFunction("Qt5Core.dll", "?qputenv@@YA_NPBDABVQByteArray@@@Z"));
    if (_qputenv == nullptr)
        return FALSE;

    DetourTransactionBegin();
    DetourAttach(&_qputenv, MyQputenv);
    DetourTransactionCommit();

    return TRUE;
}

static BOOL Detach() {
    if (_qputenv == nullptr)
        return FALSE;

    DetourTransactionBegin();
    DetourDetach(&_qputenv, MyQputenv);
    DetourTransactionCommit();

    return TRUE;
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
