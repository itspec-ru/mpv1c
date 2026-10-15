#include "mpv1c_component.h"
#include <iostream>

Mpv1cComponent::Mpv1cComponent() {
    // Конструктор по умолчанию
}

Mpv1cComponent::~Mpv1cComponent() {
    // Деструктор
}

bool CALL Mpv1cComponent::Init(void* connection) {
    m_iConnect = static_cast<AddInNative::IAddInDefBase*>(connection);
    
    if (m_iConnect) {
        WriteToLog(u"Mpv1c plugin initialized successfully");
        return true;
    }
    
    WriteToLog(u"Mpv1c plugin initialization failed: connection is null");
    return false;
}

bool CALL Mpv1cComponent::setMemManager(void* mem) {
    // Заглушка — управление памятью не используем
    return true;
}

bool CALL Mpv1cComponent::GetNProps() {
    return 0; // Нет свойств
}

bool CALL Mpv1cComponent::GetPropName(int32_t num, CHAR16** name) {
    return false; // Нет свойств
}

bool CALL Mpv1cComponent::GetNMethods() {
    return 0; // Нет методов
}

bool CALL Mpv1cComponent::GetPropVal(int32_t num, AddInNative::tVariant* value) {
    return false;
}

bool CALL Mpv1cComponent::SetPropVal(int32_t num, AddInNative::tVariant* value) {
    return false;
}

bool CALL Mpv1cComponent::CallAsProc(int32_t num, AddInNative::tVariant* params, int32_t count) {
    return false;
}

bool CALL Mpv1cComponent::CallAsFunc(int32_t num, AddInNative::tVariant* ret, AddInNative::tVariant* params, int32_t count) {
    return false;
}

void Mpv1cComponent::WriteToLog(const std::u16string& message) {
    if (m_iConnect) {
        m_iConnect->AddWarning(message.c_str());
    }
}
