cmake_minimum_required(VERSION 3.25)
project(erofs_extract_dll VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add erofs lib directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

# Define source files for the DLL
set(DLL_SOURCES
        src/erofs_extract_dll.cpp
        src/ExtractOperation.cpp
        src/ExtractHelper.cpp
        src/ErofsNode.cpp
        src/ErofsHardlinkHandle.cpp
)

# Create the DLL target
add_library(erofs_extract SHARED ${DLL_SOURCES})

# Define EROFS_EXTRACT_EXPORTS when building the DLL
target_compile_definitions(erofs_extract PRIVATE
        EROFS_EXTRACT_EXPORTS
        _FILE_OFFSET_BITS=64
        _LARGEFILE_SOURCE
        _LARGEFILE64_SOURCE
)

# Include directories
target_include_directories(erofs_extract
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Link against required libraries
target_link_libraries(erofs_extract
        PRIVATE
        erofs
        iconv
)

if(WIN32)
    target_compile_definitions(erofs_extract
            PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            _WIN32_WINNT=0x0601  # Target Windows 7 or later
    )
endif()

# Installation rules
install(TARGETS erofs_extract
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

install(FILES include/erofs_extract_dll.h
        DESTINATION include)