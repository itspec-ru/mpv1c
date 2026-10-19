#!/bin/bash
set -e

echo "=== Очистка старых сборок ==="
rm -rf build-linux-x64 build-linux-x86 build-win64 build-win32 pkg mpv1c.zip

echo "=== Building for Linux x86_64 ==="
mkdir -p build-linux-x64 && cd build-linux-x64
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
cd ..

echo "=== Building for Linux i686 (32-bit) ==="
mkdir -p build-linux-x86 && cd build-linux-x86
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/linux-i686.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
cd ..

echo "=== Building for Windows x86_64 (cross-compile) ==="
mkdir -p build-win64 && cd build-win64
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
cd ..

echo "=== Building for Windows i686 (cross-compile, 32-bit) ==="
mkdir -p build-win32 && cd build-win32
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-i686.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
cd ..

echo ""
echo "=== Формирование ZIP-архива внешней компоненты ==="

# 1. Создаем временную папку для упаковки
mkdir -p pkg

# 2. Копируем и переименовываем бинарные файлы в соответствии с соглашениями 1С
# (Убедитесь, что имена выходных файлов в CMakeLists.txt именно libmpv1c.so и mpv1c.dll)
cp build-linux-x64/libmpv1c.so pkg/libmpv1c_lin64.so
cp build-linux-x86/libmpv1c.so pkg/libmpv1c_lin32.so
cp build-win64/mpv1c.dll pkg/mpv1c_win64.dll
cp build-win32/mpv1c.dll pkg/mpv1c_win32.dll

# 3. Генерируем правильный MANIFEST.XML
cat << 'EOF' > pkg/MANIFEST.XML
<?xml version="1.0" encoding="UTF-8"?>
<bundle xmlns="http://v8.1c.ru/8.2/addin/bundle">
  <component os="Windows" arch="i386" type="native" path="mpv1c_win32.dll"/>
  <component os="Windows" arch="x86_64" type="native" path="mpv1c_win64.dll"/>
  <component os="Linux" arch="i386" type="native" path="libmpv1c_lin32.so"/>
  <component os="Linux" arch="x86_64" type="native" path="libmpv1c_lin64.so"/>
</bundle>
EOF

# 4. Создаем ZIP-архив с помощью встроенной утилиты CMake (самый надежный кроссплатформенный способ)
cd pkg
cmake -E tar cfv ../mpv1c.zip --format=zip MANIFEST.XML libmpv1c_lin32.so libmpv1c_lin64.so mpv1c_win32.dll mpv1c_win64.dll
cd ..

echo "✅ Успешно! Готовый архив mpv1c.zip создан в корне проекта."
