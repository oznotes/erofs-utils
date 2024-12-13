#ifndef EROFS_EXTRACT_DLL_H
#define EROFS_EXTRACT_DLL_H

#include <stdint.h>
#include "ExtractState.h"

#ifdef __cplusplus
extern "C" {
#endif

// Return codes
#define RET_EXTRACT_DONE              0
#define RET_EXTRACT_INIT_FAIL         1
#define RET_EXTRACT_CONFIG_FAIL       2
#define RET_EXTRACT_CREATE_DIR_FAIL   3
#define RET_EXTRACT_INIT_NODE_FAIL    4
#define RET_EXTRACT_THREAD_NUM_ERROR  5

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