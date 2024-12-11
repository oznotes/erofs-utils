#ifndef EROFS_EXTRACT_DLL_H
#define EROFS_EXTRACT_DLL_H

// Force exports by defining this before the check
#ifndef EROFS_EXTRACT_EXPORTS
#define EROFS_EXTRACT_EXPORTS
#endif

#ifdef _WIN32
    #ifdef EROFS_EXTRACT_EXPORTS
        #define EROFS_API __declspec(dllexport)
    #else
        #define EROFS_API __declspec(dllimport)
    #endif
#else
    #define EROFS_API
#endif

// Set options for extraction
typedef struct {
    bool overwrite;
    bool preserve_owner;
    bool preserve_perms;
    bool silent;
    unsigned int num_threads;
} erofs_extract_options;

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the erofs extractor with an image file
EROFS_API int erofs_extract_init(const char* image_path);

// Set output directory for extraction
EROFS_API int erofs_extract_set_outdir(const char* out_dir);

// Extract specific file or directory
EROFS_API int erofs_extract_path(const char* target_path, bool recursive);

// Extract everything from the image
EROFS_API int erofs_extract_all(void);

// Extract filesystem configuration files only
EROFS_API int erofs_extract_configs(void);

// Set options for extraction
EROFS_API int erofs_extract_set_options(const erofs_extract_options* options);

// Get the last error message
EROFS_API const char* erofs_extract_get_error(void);

// Cleanup and free resources
EROFS_API void erofs_extract_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // EROFS_EXTRACT_DLL_H