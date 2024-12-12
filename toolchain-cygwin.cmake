set(CMAKE_SYSTEM_NAME CYGWIN)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(TARGET_SYSROOT /usr/x86_64-pc-cygwin)

# Specify compilers
set(CMAKE_C_COMPILER x86_64-pc-cygwin-gcc)
set(CMAKE_CXX_COMPILER x86_64-pc-cygwin-g++)
set(CMAKE_RC_COMPILER x86_64-pc-cygwin-windres)

# Where to look for headers and libraries
set(CMAKE_FIND_ROOT_PATH ${TARGET_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# DLL specific settings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_SHARED_LIBRARY_PREFIX "")  # Remove 'lib' prefix for DLLs
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")

# Ensure proper visibility settings
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)