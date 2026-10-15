#include "addin_defines.h"
#include "mpv1c_component.h"
#include <cstring>

// Имя класса, которое будет видно в 1С
static const char16_t* CLASS_NAME = u"Mpv1c";

extern "C" {

// Возвращает список имён классов, разделённых '|'
EXPORT long CALL GetClassNames(CHAR16** names) {
    static std::u16string classNames = u"Mpv1c";
    *names = const_cast<CHAR16*>(classNames.c_str());
    return 1; // Один класс
}

// Создаёт объект класса по имени
EXPORT long CALL GetClassObject(const CHAR16* name, void** obj) {
    if (std::u16string(name) == u"Mpv1c") {
        *obj = new Mpv1cComponent();
        return 1; // Успех
    }
    return 0; // Класс не найден
}

// Уничтожает объект
EXPORT long CALL DestroyObject(void** obj) {
    if (obj && *obj) {
        delete static_cast<Mpv1cComponent*>(*obj);
        *obj = nullptr;
        return 1; // Успех
    }
    return 0;
}

} // extern "C"
