#!/bin/bash
set -e

# Значения по умолчанию
BUILD_LINUX64=false
BUILD_LINUX32=false
BUILD_WIN64=false
BUILD_WIN32=false

# Парсинг аргументов
if [ $# -eq 0 ]; then
    # Если аргументов нет, по умолчанию собираем только нативную Linux x86_64 для быстрой разработки
    BUILD_LINUX64=true
else
    for arg in "$@"; do
        case $arg in
            --linux64) BUILD_LINUX64=true ;;
            --linux32) BUILD_LINUX32=true ;;
            --win64)   BUILD_WIN64=true ;;
            --win32)   BUILD_WIN32=true ;;
            --all)
                BUILD_LINUX64=true
                BUILD_LINUX32=true
                BUILD_WIN64=true
                BUILD_WIN32=true
                ;;
            *)
                echo "❌ Неизвестный параметр: $arg"
                echo "Использование: $0 [--linux64] [--linux32] [--win64] [--win32] [--all]"
                exit 1
                ;;
        esac
    done
fi

echo "=== Очистка старых сборок ==="
rm -rf build-linux-x64 build-linux-x86 build-win64 build-win32 pkg mpv1c.zip

# Функция для сборки целевой платформы
build_target() {
    local name=$1
    local dir=$2
    local toolchain=$3

    echo "=== Building for $name ==="
    mkdir -p "$dir" && cd "$dir"
    if [ -n "$toolchain" ]; then
        cmake .. -DCMAKE_TOOLCHAIN_FILE="../$toolchain" -DCMAKE_BUILD_TYPE=Release
    else
        cmake .. -DCMAKE_BUILD_TYPE=Release
    fi
    cmake --build .
    cd ..
}

# Выполняем сборку только выбранных платформ
if [ "$BUILD_LINUX64" = true ]; then
    build_target "Linux x86_64" "build-linux-x64" ""
fi

if [ "$BUILD_LINUX32" = true ]; then
    build_target "Linux i686 (32-bit)" "build-linux-x86" "cmake/linux-i686.cmake"
fi

if [ "$BUILD_WIN64" = true ]; then
    build_target "Windows x86_64 (cross-compile)" "build-win64" "cmake/mingw-w64-x86_64.cmake"
fi

if [ "$BUILD_WIN32" = true ]; then
    build_target "Windows i686 (cross-compile, 32-bit)" "build-win32" "cmake/mingw-w64-i686.cmake"
fi

echo ""
echo "=== Формирование ZIP-архива внешней компоненты ==="

# Проверка, было ли собрано хоть что-то
if [ "$BUILD_LINUX64" = false ] && [ "$BUILD_LINUX32" = false ] && [ "$BUILD_WIN64" = false ] && [ "$BUILD_WIN32" = false ]; then
    echo "⚠️ Ни одна платформа не выбрана для сборки. ZIP-архив не создан."
    exit 0
fi

mkdir -p pkg

# Динамическое формирование MANIFEST.XML и списка файлов для ZIP
MANIFEST_CONTENT='<?xml version="1.0" encoding="UTF-8"?>
<bundle xmlns="http://v8.1c.ru/8.2/addin/bundle">'

ZIP_FILES=""

if [ "$BUILD_LINUX64" = true ] && [ -f "build-linux-x64/libmpv1c.so" ]; then
    cp build-linux-x64/libmpv1c.so pkg/libmpv1c_lin64.so
    MANIFEST_CONTENT="$MANIFEST_CONTENT
  <component os=\"Linux\" arch=\"x86_64\" type=\"native\" path=\"libmpv1c_lin64.so\"/>"
    ZIP_FILES="$ZIP_FILES libmpv1c_lin64.so"
fi

if [ "$BUILD_LINUX32" = true ] && [ -f "build-linux-x86/libmpv1c.so" ]; then
    cp build-linux-x86/libmpv1c.so pkg/libmpv1c_lin32.so
    MANIFEST_CONTENT="$MANIFEST_CONTENT
  <component os=\"Linux\" arch=\"i386\" type=\"native\" path=\"libmpv1c_lin32.so\"/>"
    ZIP_FILES="$ZIP_FILES libmpv1c_lin32.so"
fi

if [ "$BUILD_WIN64" = true ] && [ -f "build-win64/mpv1c.dll" ]; then
    cp build-win64/mpv1c.dll pkg/mpv1c_win64.dll
    MANIFEST_CONTENT="$MANIFEST_CONTENT
  <component os=\"Windows\" arch=\"x86_64\" type=\"native\" path=\"mpv1c_win64.dll\"/>"
    ZIP_FILES="$ZIP_FILES mpv1c_win64.dll"
fi

if [ "$BUILD_WIN32" = true ] && [ -f "build-win32/mpv1c.dll" ]; then
    cp build-win32/mpv1c.dll pkg/mpv1c_win32.dll
    MANIFEST_CONTENT="$MANIFEST_CONTENT
  <component os=\"Windows\" arch=\"i386\" type=\"native\" path=\"mpv1c_win32.dll\"/>"
    ZIP_FILES="$ZIP_FILES mpv1c_win32.dll"
fi

MANIFEST_CONTENT="$MANIFEST_CONTENT
</bundle>"

echo "$MANIFEST_CONTENT" > pkg/MANIFEST.XML
ZIP_FILES="MANIFEST.XML $ZIP_FILES"

# Создаем ZIP-архив
cd pkg
# Убираем лишние пробелы из списка файлов
ZIP_FILES=$(echo $ZIP_FILES | xargs)
cmake -E tar cfv ../mpv1c.zip --format=zip $ZIP_FILES
cd ..

echo "✅ Успешно! Готовый архив mpv1c.zip создан в корне проекта."
echo "   Включает платформы: $(echo $ZIP_FILES | tr ' ' '\n' | grep -v MANIFEST.XML | sed 's/libmpv1c_//; s/mpv1c_//' | tr '\n' ' ')"
