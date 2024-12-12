set(TARGET_extract_dll "erofs_extract")
set(TARGET_SRC_DIR "${PROJECT_ROOT_DIR}/extract")

# DLL-specific CMake configuration
add_library(${TARGET_extract_dll} SHARED
    ${TARGET_SRC_DIR}/erofs_extract_dll.cpp
    ${TARGET_SRC_DIR}/ExtractOperation.cpp
    ${TARGET_SRC_DIR}/ExtractHelper.cpp
    ${TARGET_SRC_DIR}/ErofsNode.cpp
    ${TARGET_SRC_DIR}/ErofsHardlinkHandle.cpp
)

target_include_directories(${TARGET_extract_dll} PRIVATE
    "${TARGET_SRC_DIR}/include"
    ${common_headers}
)

target_compile_definitions(${TARGET_extract_dll} PRIVATE 
    EROFS_EXTRACT_EXPORTS
    _FILE_OFFSET_BITS=64
    _LARGEFILE_SOURCE
    _LARGEFILE64_SOURCE
    CYGWIN
)

# Control symbol visibility
target_compile_options(${TARGET_extract_dll} PRIVATE
    -fvisibility=hidden
    -fvisibility-inlines-hidden
)

target_link_libraries(${TARGET_extract_dll} PRIVATE
    ${common_static_link_lib}
)

# Windows/Cygwin specific linker flags for controlled exports
target_link_options(${TARGET_extract_dll} PRIVATE
    -Wl,--exclude-all-symbols
    -Wl,--export-all-symbols
    -Wl,--enable-auto-import
    -Wl,--enable-runtime-pseudo-reloc
)

# Ensure proper DLL naming
set_target_properties(${TARGET_extract_dll} PROPERTIES 
    PREFIX ""
    OUTPUT_NAME "cygerofs_extract"
    WINDOWS_EXPORT_ALL_SYMBOLS OFF  # Turn this off since we're controlling exports explicitly
)

# Installation
install(TARGETS ${TARGET_extract_dll}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
