#ifndef EROFS_EXTRACT_DLL_H
#define EROFS_EXTRACT_DLL_H

#ifdef _WIN32
#ifdef EROFS_EXTRACT_EXPORTS
        #define EROFS_API __declspec(dllexport) __attribute__((used)) __attribute__((visibility("default")))
    #else
        #define EROFS_API __declspec(dllimport)
    #endif
#else
#define EROFS_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Options struct for extraction configuration
typedef struct erofs_extract_options {
    bool overwrite;
    bool preserve_owner;
    bool preserve_perms;
    bool silent;
    unsigned int num_threads;
} erofs_extract_options;

// Return codes
#define RET_EXTRACT_DONE 0
#define RET_EXTRACT_ERROR -1

// API Functions - note the EROFS_API prefix
EROFS_API int __attribute__((used)) erofs_extract_init(const char* image_path);
EROFS_API int __attribute__((used)) erofs_extract_set_outdir(const char* out_dir);
EROFS_API int __attribute__((used)) erofs_extract_path(const char* target_path, bool recursive);
EROFS_API int __attribute__((used)) erofs_extract_all(void);
EROFS_API int __attribute__((used)) erofs_extract_configs(void);
EROFS_API int __attribute__((used)) erofs_extract_set_options(const erofs_extract_options* options);
EROFS_API const char* __attribute__((used)) erofs_extract_get_error(void);
EROFS_API void __attribute__((used)) erofs_extract_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // EROFS_EXTRACT_DLL_H