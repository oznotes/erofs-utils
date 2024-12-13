set(TARGET_extract_dll "erofs_extract")
set(TARGET_SRC_DIR "${PROJECT_ROOT_DIR}/extract")

add_library(${TARGET_extract_dll} SHARED
    ${TARGET_SRC_DIR}/erofs_extract_dll.cpp
    ${TARGET_SRC_DIR}/erofs_extract_impl.cpp
)

target_include_directories(${TARGET_extract_dll} PRIVATE
    "${TARGET_SRC_DIR}/include"
    ${common_headers}
    ${PROJECT_ROOT_DIR}/include
)

target_compile_definitions(${TARGET_extract_dll} PRIVATE
    EROFS_EXTRACT_EXPORTS=1
    _FILE_OFFSET_BITS=64
    _LARGEFILE_SOURCE
    _LARGEFILE64_SOURCE
    CYGWIN
    _DEBUG
)

target_compile_options(${TARGET_extract_dll} PRIVATE
    -fvisibility=hidden
    -g
    -O0
    -Wall
    -Wextra
    -fno-exceptions
    -fno-rtti
)

target_link_libraries(${TARGET_extract_dll} PRIVATE
    ${common_static_link_lib}
    erofs_static
)

set_target_properties(${TARGET_extract_dll} PROPERTIES
    PREFIX ""
    OUTPUT_NAME "cygerofs_extract"
    LINK_FLAGS "-Wl,--exclude-all-symbols -Wl,--enable-auto-import"
    DEBUG_POSTFIX "d"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

install(TARGETS ${TARGET_extract_dll}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)