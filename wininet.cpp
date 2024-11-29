#include "stdafx.h"

static HMODULE _wininet = nullptr;
static decltype(InternetGetConnectedState)* _realInternetGetConnectedState = nullptr;
static std::once_flag _wininet_flag;

static void InitWininet() {
    _wininet = LoadLibraryW(L"C:\\Windows\\system32\\wininet.dll");
    if (_wininet == nullptr)
        return;
    _realInternetGetConnectedState = reinterpret_cast<decltype(InternetGetConnectedState)*>(GetProcAddress(_wininet, "InternetGetConnectedState"));
    if (_realInternetGetConnectedState == nullptr)
        return;
}

#if !defined(_WIN64)
#pragma comment(linker, "/EXPORT:InternetGetConnectedState=_InternetGetConnectedState@8")
#endif
EXTERN_C __declspec(dllexport) _Success_(return != FALSE) BOOL STDAPICALLTYPE InternetGetConnectedState(
    _Out_  LPDWORD  lpdwFlags,
    _Reserved_ DWORD dwReserved) {
    std::call_once(_wininet_flag, InitWininet);
    if (_realInternetGetConnectedState) {
        return _realInternetGetConnectedState(lpdwFlags, dwReserved);
    }
    else {
        *lpdwFlags = INTERNET_CONNECTION_OFFLINE;
        return FALSE;
    }
}
