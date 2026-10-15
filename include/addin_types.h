#pragma once

#include <cstdint>
#include "addin_defines.h"  // Для CHAR16

namespace AddInNative {

// Перечисления Native API
enum class eValueTypes : int32_t {
    VT_EMPTY = 0,
    VT_NULL,
    VT_I4,
    VT_R8,
    VT_BOOL,
    VT_STRING,
    VT_DATE,
    VT_BLOB,
    VT_DISPATCH,
    VT_ERROR,
    VT_VARIANT
};

enum class ePropAccessType : int32_t {
    READ = 1,
    WRITE = 2,
    READ_WRITE = 3
};

// Структура значения (упрощённая)
struct tVariant {
    eValueTypes vt;
    union {
        int32_t     lVal;
        double      dblVal;
        bool        bVal;
        CHAR16*     pwstrVal;
        uint32_t    ulVal;
        void*       pdispVal;
    };
    uint32_t wReserved;
};

using tVariantReturn = tVariant;

} // namespace AddInNative
