set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Set the standard MinGW-w64 GCC compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)

# Point to the MinGW-w64 sysroot installed on your system
set(CMAKE_SYSROOT /usr/x86_64-w64-mingw32)

# Set search paths
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Force static linking for the executable to avoid missing DLL errors on Windows
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static -static-libgcc -Wl,-Bstatic")
