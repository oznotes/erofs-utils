###############################------extract.erofs------###############################
# Define both DLL and executable targets
set(TARGET_extract_dll "erofs_extract")
set(TARGET_extract "extract.erofs")

set(TARGET_SRC_DIR "${PROJECT_ROOT_DIR}/extract")

# First, create the DLL source and header files
file(WRITE "${TARGET_SRC_DIR}/erofs_extract_dll.h"
"#ifndef EROFS_EXTRACT_DLL_H
#define EROFS_EXTRACT_DLL_H
// ... [content of erofs_extract_dll.h] ...
#endif")

file(WRITE "${TARGET_SRC_DIR}/erofs_extract_dll.cpp"
"#include \"erofs_extract_dll.h\"
// ... [content of erofs_extract_dll.cpp] ...")

# Collect source files but exclude main.cpp for the DLL
file(GLOB extract_lib_srcs 
    "${TARGET_SRC_DIR}/*.cpp"
)
list(FILTER extract_lib_srcs EXCLUDE REGEX ".*main\\.cpp$")
list(APPEND extract_lib_srcs "${TARGET_SRC_DIR}/erofs_extract_dll.cpp")

# Build the DLL
add_library(${TARGET_extract_dll} SHARED ${extract_lib_srcs})
target_include_directories(${TARGET_extract_dll} 
    PUBLIC
        "${TARGET_SRC_DIR}"  # Include directory for all headers
    PRIVATE
        ${common_headers}
)
target_link_libraries(${TARGET_extract_dll} ${common_static_link_lib})
target_compile_options(${TARGET_extract_dll} PRIVATE ${common_compile_flags} "-Wno-unused-result")
target_compile_definitions(${TARGET_extract_dll} PRIVATE EROFS_EXTRACT_EXPORTS)

# Build the executable
file(GLOB extract_exe_srcs "${TARGET_SRC_DIR}/*.cpp")
add_executable(${TARGET_extract} ${extract_exe_srcs})
target_include_directories(${TARGET_extract} PRIVATE
        "${TARGET_SRC_DIR}/include"
        ${common_headers}
)
target_link_libraries(${TARGET_extract} ${common_static_link_lib})
target_compile_options(${TARGET_extract} PRIVATE ${common_compile_flags} "-Wno-unused-result")