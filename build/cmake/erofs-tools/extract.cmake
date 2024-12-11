###############################------extract.erofs------###############################
set(TARGET_extract_dll "erofs_extract")
set(TARGET_extract "extract.erofs")

set(TARGET_SRC_DIR "${PROJECT_ROOT_DIR}/extract")

# Collect source files but exclude main.cpp for the DLL
file(GLOB extract_lib_srcs 
    "${TARGET_SRC_DIR}/*.cpp"
)
list(FILTER extract_lib_srcs EXCLUDE REGEX ".*main\\.cpp$")

# Build the DLL
add_library(${TARGET_extract_dll} SHARED ${extract_lib_srcs})
target_include_directories(${TARGET_extract_dll} 
    PUBLIC
        "${TARGET_SRC_DIR}/include"
    PRIVATE
        "${TARGET_SRC_DIR}"
        ${common_headers}
)
target_link_libraries(${TARGET_extract_dll} ${common_static_link_lib})
target_compile_options(${TARGET_extract_dll} PRIVATE 
    ${common_compile_flags} 
    "-Wno-unused-result"
    "-fno-exceptions"  # Disable exceptions
)
target_compile_definitions(${TARGET_extract_dll} PRIVATE 
    EROFS_EXTRACT_EXPORTS
    _GNU_SOURCE
)

# Original executable build
file(GLOB extract_exe_srcs "${TARGET_SRC_DIR}/*.cpp")
add_executable(${TARGET_extract} ${extract_exe_srcs})
target_include_directories(${TARGET_extract} PRIVATE
        "${TARGET_SRC_DIR}/include"
        ${common_headers}
)
target_link_libraries(${TARGET_extract} ${common_static_link_lib})
target_compile_options(${TARGET_extract} PRIVATE ${common_compile_flags} "-Wno-unused-result")