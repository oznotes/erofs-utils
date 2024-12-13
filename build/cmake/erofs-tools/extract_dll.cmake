set(TARGET_extract_dll "erofs_extract")
set(TARGET_SRC_DIR "${PROJECT_ROOT_DIR}/extract")

# DLL-specific CMake configuration
add_library(${TARGET_extract_dll} SHARED
        ${TARGET_SRC_DIR}/erofs_extract_dll.cpp
)

target_include_directories(${TARGET_extract_dll} PRIVATE
        "${TARGET_SRC_DIR}/include"
        ${common_headers}
)

# Add debug definitions
target_compile_definitions(${TARGET_extract_dll} PRIVATE
        EROFS_EXTRACT_EXPORTS=1
        _FILE_OFFSET_BITS=64
        _LARGEFILE_SOURCE
        _LARGEFILE64_SOURCE
        CYGWIN
        _DEBUG
)

# Compiler options
target_compile_options(${TARGET_extract_dll} PRIVATE
        -fvisibility=hidden
        -g  # Add debug symbols
        -O0 # No optimization for better debugging
        -Wall
        -Wextra
)

# Link with required libraries
target_link_libraries(${TARGET_extract_dll} PRIVATE
        ${common_static_link_lib}
)

# DLL specific settings
set_target_properties(${TARGET_extract_dll} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "cygerofs_extract"
        LINK_FLAGS "-Wl,--exclude-all-symbols -Wl,--enable-auto-import"
        DEBUG_POSTFIX "d"  # Adds 'd' suffix for debug builds
)

# Installation
install(TARGETS ${TARGET_extract_dll}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
)

# Generate PDB files for debugging on Windows
if(MSVC)
    set_target_properties(${TARGET_extract_dll} PROPERTIES 
        COMPILE_PDB_NAME "${TARGET_extract_dll}"
        COMPILE_PDB_OUTPUT_DIR "${CMAKE_BINARY_DIR}"
    )
endif()