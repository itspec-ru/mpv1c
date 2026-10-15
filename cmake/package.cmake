cmake_minimum_required(VERSION 3.16)

set(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(PKG_DIR "${PROJECT_ROOT}/pkg")
set(ZIP_OUTPUT "${PROJECT_ROOT}/mpv1c.zip")

message(STATUS "=== Подготовка пакета mpv1c ===")

# Очищаем и создаем каталог pkg
file(REMOVE_RECURSE "${PKG_DIR}")
file(MAKE_DIRECTORY "${PKG_DIR}")

# Определяем пути к собранным библиотекам
set(LIB_LINUX_64 "${PROJECT_ROOT}/build-linux-x64/libmpv1c.so")
set(LIB_LINUX_32 "${PROJECT_ROOT}/build-linux-x86/libmpv1c.so")
set(LIB_WIN_64 "${PROJECT_ROOT}/build-win64/mpv1c.dll")
set(LIB_WIN_32 "${PROJECT_ROOT}/build-win32/mpv1c.dll")

# Проверяем наличие всех файлов
foreach(LIB ${LIB_LINUX_64} ${LIB_LINUX_32} ${LIB_WIN_64} ${LIB_WIN_32})
    if(NOT EXISTS "${LIB}")
        message(FATAL_ERROR "Не найден файл: ${LIB}")
    endif()
endforeach()

# Копируем библиотеки с правильными именами
message(STATUS "Копирование библиотек...")

# Linux 64-bit
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${LIB_LINUX_64}" "${PKG_DIR}/libmpv1c_lin64.so"
)

# Linux 32-bit
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${LIB_LINUX_32}" "${PKG_DIR}/libmpv1c_lin32.so"
)

# Windows 64-bit
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${LIB_WIN_64}" "${PKG_DIR}/mpv1c_win64.dll"
)

# Windows 32-bit
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${LIB_WIN_32}" "${PKG_DIR}/mpv1c_win32.dll"
)

# Создаем MANIFEST.XML
message(STATUS "Создание MANIFEST.XML...")
file(WRITE "${PKG_DIR}/MANIFEST.XML" "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<bundle xmlns=\"http://v8.1c.ru/8.2/addin/bundle\">
  <component os=\"Windows\" arch=\"i386\" type=\"native\" path=\"mpv1c_win32.dll\"/>
  <component os=\"Windows\" arch=\"x86_64\" type=\"native\" path=\"mpv1c_win64.dll\"/>
  <component os=\"Linux\" arch=\"i386\" type=\"native\" path=\"libmpv1c_lin32.so\"/>
  <component os=\"Linux\" arch=\"x86_64\" type=\"native\" path=\"libmpv1c_lin64.so\"/>
</bundle>
")

# Создаем ZIP-архив
message(STATUS "Создание ZIP-архива...")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cfv "${ZIP_OUTPUT}" --format=zip 
            MANIFEST.XML libmpv1c_lin32.so libmpv1c_lin64.so mpv1c_win32.dll mpv1c_win64.dll
    WORKING_DIRECTORY "${PKG_DIR}"
    RESULT_VARIABLE ZIP_RESULT
)

if(ZIP_RESULT EQUAL 0)
    message(STATUS "✅ Успешно создан архив: ${ZIP_OUTPUT}")
    message(STATUS "Содержимое архива:")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar tvf "${ZIP_OUTPUT}")
else()
    message(FATAL_ERROR "❌ Ошибка при создании ZIP-архива")
endif()
