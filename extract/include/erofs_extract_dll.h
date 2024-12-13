#ifndef EROFS_EXTRACT_DLL_H
#define EROFS_EXTRACT_DLL_H

#include <stdint.h>
#include "ExtractState.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #define EROFS_API __declspec(dllexport)
    #define EROFS_CALL __cdecl
#else
    #define EROFS_API
    #define EROFS_CALL
#endif

typedef struct erofs_extract_options {
    bool overwrite;
    bool silent;
    unsigned int num_threads;
    int debug_level;
} erofs_extract_options;

// API Functions
EROFS_API int EROFS_CALL erofs_extract_init(const char* image_path);
EROFS_API int EROFS_CALL erofs_extract_set_outdir(const char* out_dir);
EROFS_API int EROFS_CALL erofs_extract_path(const char* target_path, bool recursive);
EROFS_API int EROFS_CALL erofs_extract_all(void);
EROFS_API int EROFS_CALL erofs_extract_set_options(const erofs_extract_options* options);
EROFS_API const char* EROFS_CALL erofs_extract_get_error(void);
EROFS_API void EROFS_CALL erofs_extract_cleanup(void);

// Test functions
EROFS_API int EROFS_CALL erofs_test_simple(void);
EROFS_API int EROFS_CALL erofs_test_function(int input, int* output);

#ifdef __cplusplus
}
#endif

#endif // EROFS_EXTRACT_DLL_H