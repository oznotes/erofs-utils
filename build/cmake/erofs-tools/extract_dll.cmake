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

target_compile_options(${TARGET_extract_dll} PRIVATE
        -fvisibility=hidden
        -fdata-sections
        -ffunction-sections
)

target_link_libraries(${TARGET_extract_dll} PRIVATE
        ${common_static_link_lib}
)

# Configure exports and DLL properties
set_target_properties(${TARGET_extract_dll} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "cygerofs_extract"
        LINK_FLAGS "-Wl,--export-dynamic -Wl,--enable-auto-import"
)

# Installation
install(TARGETS ${TARGET_extract_dll}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
)