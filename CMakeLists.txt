cmake_minimum_required(VERSION 4.1)
project(IO_lab2 C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(USE_NATIVE_ARCH "Compile with -march=native" ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -fomit-frame-pointer)
    if(USE_NATIVE_ARCH)
        add_compile_options(-march=native)
    else()
        add_compile_options(-mtune=generic)
    endif()
endif()

find_package(OpenMP REQUIRED)

add_executable(${PROJECT_NAME}
    src/tui.c
    src/zad1.c
    src/zad2.c
    src/zad3.c
    src/zad4.c
    src/image_operations.c
    src/main.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

target_include_directories(${PROJECT_NAME} PRIVATE inc)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pipe)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    message(STATUS "IPO/LTO is supported. Enabling for Release build.")
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
else()
    message(WARNING "IPO/LTO is not supported: ${ipo_error}")
endif()

if(WIN32)

    target_compile_options(${PROJECT_NAME} PRIVATE -fopenmp)

    target_link_libraries(${PROJECT_NAME} PRIVATE 
        -static
        -static-libgcc
        -lgomp 
        -lwinpthread 
        -m
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_C m)
endif()
