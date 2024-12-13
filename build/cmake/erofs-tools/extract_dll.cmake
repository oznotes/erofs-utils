set(TARGET_extract_dll "erofs_extract")

add_library(${TARGET_extract_dll} SHARED
    ${PROJECT_ROOT_DIR}/extract/erofs_extract_dll.cpp
    ${PROJECT_ROOT_DIR}/extract/ExtractOperation.cpp
    ${PROJECT_ROOT_DIR}/extract/ExtractHelper.cpp
    ${PROJECT_ROOT_DIR}/extract/ErofsNode.cpp
    ${PROJECT_ROOT_DIR}/extract/ErofsHardlinkHandle.cpp
)

target_include_directories(${TARGET_extract_dll} PRIVATE
    ${PROJECT_ROOT_DIR}/extract/include
    ${PROJECT_ROOT_DIR}/include
    ${common_headers}
)

target_link_libraries(${TARGET_extract_dll} PRIVATE
    erofs_static
    ${common_static_link_lib}
)

target_compile_definitions(${TARGET_extract_dll} PRIVATE
    EROFS_EXTRACT_EXPORTS
    _FILE_OFFSET_BITS=64
    _LARGEFILE_SOURCE
    _LARGEFILE64_SOURCE
    CYGWIN
)

# Set DLL properties
set_target_properties(${TARGET_extract_dll} PROPERTIES
    PREFIX "cyg"
    OUTPUT_NAME "erofs_extract"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/erofs-tools"
)

install(TARGETS ${TARGET_extract_dll}
    RUNTIME DESTINATION bin
)