#ifndef EROFS_EXTRACT_DLL_H
#define EROFS_EXTRACT_DLL_H

#ifdef _WIN32
#ifdef EROFS_EXTRACT_EXPORTS
        #define EROFS_API __declspec(dllexport)
    #else
        #define EROFS_API __declspec(dllimport)
    #endif
#else
#define EROFS_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct erofs_extract_options {
    bool overwrite;
    bool preserve_owner;
    bool preserve_perms;
    bool silent;
    unsigned int num_threads;
} erofs_extract_options;

// API Functions
EROFS_API int erofs_extract_init(const char* image_path);
EROFS_API int erofs_extract_set_outdir(const char* out_dir);
EROFS_API int erofs_extract_path(const char* target_path, bool recursive);
EROFS_API int erofs_extract_all(void);
EROFS_API int erofs_extract_configs(void);
EROFS_API int erofs_extract_set_options(const erofs_extract_options* options);
EROFS_API const char* erofs_extract_get_error(void);
EROFS_API void erofs_extract_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // EROFS_EXTRACT_DLL_H