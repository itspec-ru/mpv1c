#pragma once

#include "addin_interfaces.h"
#include <string>

class Mpv1cComponent : 
    public AddInNative::IInitDoneBase,
    public AddInNative::ILangExtenderBase,
    public AddInNative::IDispatchBase 
{
public:
    Mpv1cComponent();
    ~Mpv1cComponent() override;

    // IInitDoneBase
    bool CALL Init(void* connection) override;
    bool CALL setMemManager(void* mem) override;

    // ILangExtenderBase + IDispatchBase (методы совпадают)
    bool CALL GetNProps() override;
    bool CALL GetPropName(int32_t num, CHAR16** name) override;
    bool CALL GetNMethods() override;
    bool CALL GetPropVal(int32_t num, AddInNative::tVariant* value) override;
    bool CALL SetPropVal(int32_t num, AddInNative::tVariant* value) override;
    bool CALL CallAsProc(int32_t num, AddInNative::tVariant* params, int32_t count) override;
    bool CALL CallAsFunc(int32_t num, AddInNative::tVariant* ret, AddInNative::tVariant* params, int32_t count) override;

private:
    AddInNative::IAddInDefBase* m_iConnect = nullptr;
    void WriteToLog(const std::u16string& message);
};
