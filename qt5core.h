#pragma once

class QByteArray;
using qputenv_t = bool (*__cdecl)(char const*, QByteArray*);
using QByteArray_new_t = void (*__fastcall)(QByteArray*, void*, char const*, int);
using QByteArray_delete_t = void (*__fastcall)(QByteArray*, void*);

extern qputenv_t _qputenv;
bool __cdecl MyQputenv(char const* varName, QByteArray* value);
