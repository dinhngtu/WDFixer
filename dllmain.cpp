#include "stdafx.h"
#include "qt5core.h"
#include "detours.h"

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
        return Attach();
    case DLL_PROCESS_DETACH:
        return Detach();
    }
}
