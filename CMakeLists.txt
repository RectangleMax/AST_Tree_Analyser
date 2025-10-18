cmake_minimum_required(VERSION 3.25)
project(analyser VERSION 1.0.0 LANGUAGES CXX)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Флаги отладки
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fno-omit-frame-pointer")

# Configure local githooks
execute_process(
    COMMAND git config core.hooksPath .githooks
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_COMMAND_RESULT
)

# Поиск зависимостей
find_package(Boost REQUIRED COMPONENTS program_options)

# Ручной поиск range-v3
find_path(RANGE_V3_INCLUDE_DIR
    NAMES range/v3/all.hpp
    PATHS /usr/include /usr/local/include
)

if(NOT RANGE_V3_INCLUDE_DIR)
    message(FATAL_ERROR "range-v3 not found!")
endif()

# Единый исполняемый файл со всеми исходниками
add_executable(analyser
    # Основной файл
    main.cpp
    
    # Исходники из src/
    src/function.cpp
    src/file.cpp
    src/metric.cpp
    src/metric_accumulator.cpp
    src/cmd_options.cpp
    
    # Исходники из поддиректорий
    src/metric_impl/code_lines_count.cpp
    src/metric_impl/cyclomatic_complexity.cpp
    src/metric_impl/parameters_count.cpp
    src/metric_accumulator_impl/average_accumulator.cpp
    src/metric_accumulator_impl/sum_average_accumulator.cpp
)

# Директории включения
target_include_directories(analyser PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${RANGE_V3_INCLUDE_DIR}
)

# Флаги компиляции
target_compile_options(analyser PRIVATE 
    -g -O0 -fno-omit-frame-pointer
)

# Связывание с внешними библиотеками
target_link_libraries(analyser PRIVATE
    Boost::program_options
)

# Настройка тестов
include(CTest)
enable_testing()

# Цель для полной очистки
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Removing entire build directory"
)