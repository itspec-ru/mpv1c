#pragma once

// Макросы экспорта для кроссплатформенности
#if defined(_WIN32) || defined(_WIN64)
    #define EXPORT __declspec(dllexport)
    #define CALL __stdcall
#else
    #define EXPORT __attribute__((visibility("default")))
    #define CALL
#endif

// Типы символов Native API 1С
using CHAR16 = char16_t;
