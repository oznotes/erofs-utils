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

# Hide all symbols by default
target_compile_options(${TARGET_extract_dll} PRIVATE
        -fvisibility=hidden
        -fdata-sections
        -ffunction-sections
)

target_link_libraries(${TARGET_extract_dll} PRIVATE
        ${common_static_link_lib}
)

# Configure the .def file path
set(DEF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/erofs_extract.def")

# Use def file and strict export control
set_target_properties(${TARGET_extract_dll} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "cygerofs_extract"
        LINK_FLAGS "-Wl,--kill-at -Wl,--enable-auto-import -Wl,--def,${DEF_FILE}"
)

# Installation
install(TARGETS ${TARGET_extract_dll}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
)

# Add a message to help debug the path
message(STATUS "DEF file path: ${DEF_FILE}")