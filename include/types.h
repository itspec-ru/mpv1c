#ifndef __TYPES_H__
#define __TYPES_H__

#include <cstdint>

#if defined(__linux__) || defined(__APPLE__)
    #define LINUX_OR_MAC
#endif

#ifndef __cdecl
    #if defined(__GNUC__)
        #define __cdecl __attribute__((cdecl))
    #endif
#endif

#if defined(LINUX_OR_MAC)
    #define ADDIN_API __attribute__((visibility("default")))
    typedef unsigned short WCHAR_T;
#else
    #define ADDIN_API __stdcall __declspec(dllexport)
    typedef wchar_t WCHAR_T;
#endif

enum typesEnum {
    VTYPE_EMPTY = 0, VTYPE_NULL, VTYPE_I2, VTYPE_I4, VTYPE_R4, VTYPE_R8,
    VTYPE_DATE, VTYPE_TM, VTYPE_PSTR, VTYPE_PWSTR, VTYPE_ERROR, VTYPE_BOOL,
    VTYPE_VARIANT, VTYPE_I1, VTYPE_UI1, VTYPE_I2INT, VTYPE_I4INT,
    VTYPE_R4INT, VTYPE_R8INT, VTYPE_STR = 250, VTYPE_BLOB = 251
};

struct tVariant {
    union {
        int8_t    i8val;      int16_t   shortVal;   int32_t   longVal;
        float     fltVal;     double    dblVal;     bool      boolVal;
        char*     pstrVal;    WCHAR_T*  pwstrVal;   void*     pinterfaceVal;
        int64_t   int64Val;   uint8_t   uint8Val;   uint16_t  ushortVal;
        uint32_t  ulongVal;   uint64_t  uint64Val;
    };
    uint32_t  cbstrVal;
    typesEnum vt;
};

#endif // __TYPES_H__
