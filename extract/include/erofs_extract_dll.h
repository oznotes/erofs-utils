#ifndef EROFS_EXTRACT_DLL_H
#define EROFS_EXTRACT_DLL_H

#include <stdint.h>
#include "ExtractState.h"
#include "ExtractCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct erofs_extract_options {
    bool overwrite;
    bool silent;
    unsigned int num_threads;
    int debug_level;
} erofs_extract_options;

// API Functions
int erofs_extract_init(const char* image_path);
int erofs_extract_set_outdir(const char* out_dir);
int erofs_extract_path(const char* target_path, bool recursive);
int erofs_extract_all(void);
int erofs_extract_set_options(const erofs_extract_options* options);
const char* erofs_extract_get_error(void);
void erofs_extract_cleanup(void);

// Test functions
int erofs_test_simple(void);
int erofs_test_function(int input, int* output);

#ifdef __cplusplus
}
#endif

#endif // EROFS_EXTRACT_DLL_H