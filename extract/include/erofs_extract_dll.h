#ifndef EROFS_EXTRACT_DLL_H
#define EROFS_EXTRACT_DLL_H

// Force exports by defining this during DLL build
#define EROFS_EXTRACT_EXPORTS

#ifdef _WIN32
#ifdef EROFS_EXTRACT_EXPORTS
        #define EROFS_API __declspec(dllexport) __attribute__((used))
    #else
        #define EROFS_API __declspec(dllimport)
    #endif
#else
#define EROFS_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Keep existing struct definition
typedef struct {
    bool overwrite;
    bool preserve_owner;
    bool preserve_perms;
    bool silent;
    unsigned int num_threads;
} erofs_extract_options;

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