# Полная очистка
cd build
rm -rf *

# Конфигурация с подробным выводом
cmake -DCMAKE_VERBOSE_MAKEFILE=ON ..

# Сборка
make analyser

# Проверка отладочной информации
objdump -g src/CMakeFiles/file.dir/file.cpp.o | grep -A5 -B5 "file.cpp"
readelf -p .debug_str ./analyser | grep file.cpp