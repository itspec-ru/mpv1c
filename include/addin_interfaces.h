#pragma once

#include "addin_types.h"
#include "addin_defines.h"

namespace AddInNative {

// Интерфейс для обращения к 1С (запись в журнал, ошибки и т.д.)
class IAddInDefBase {
public:
    virtual bool CALL Connect(void* pConnection) = 0;
    virtual bool CALL Disconnect() = 0;
    virtual bool CALL AddError(const CHAR16* message) = 0;
    virtual bool CALL AddWarning(const CHAR16* message) = 0;
    // ... остальные методы можно добавить позже
    virtual ~IAddInDefBase() = default;
};

// Интерфейс инициализации
class IInitDoneBase {
public:
    virtual bool CALL Init(void* connection) = 0;
    virtual bool CALL setMemManager(void* mem) = 0;
    virtual ~IInitDoneBase() = default;
};

// Интерфейс локализации (заглушка)
class ILangExtenderBase {
public:
    virtual bool CALL GetNProps() = 0;
    virtual bool CALL GetPropName(int32_t num, CHAR16** name) = 0;
    virtual ~ILangExtenderBase() = default;
};

// Интерфейс вызова методов из 1С
class IDispatchBase {
public:
    virtual bool CALL GetNProps() = 0;
    virtual bool CALL GetNMethods() = 0;
    virtual bool CALL GetPropVal(int32_t num, tVariant* value) = 0;
    virtual bool CALL SetPropVal(int32_t num, tVariant* value) = 0;
    virtual bool CALL CallAsProc(int32_t num, tVariant* params, int32_t count) = 0;
    virtual bool CALL CallAsFunc(int32_t num, tVariant* ret, tVariant* params, int32_t count) = 0;
    virtual ~IDispatchBase() = default;
};

} // namespace AddInNative
