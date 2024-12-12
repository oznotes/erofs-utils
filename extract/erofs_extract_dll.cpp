#include "erofs_extract_dll.h"
#include "erofs_extract_impl.h"

EROFS_API int erofs_extract_init(const char* image_path) {
    return erofs_extract_init_impl(image_path);
}

EROFS_API int erofs_extract_set_outdir(const char* out_dir) {
    return erofs_extract_set_outdir_impl(out_dir);
}

EROFS_API int erofs_extract_path(const char* target_path, bool recursive) {
    return erofs_extract_path_impl(target_path, recursive);
}

EROFS_API int erofs_extract_all(void) {
    return erofs_extract_all_impl();
}

EROFS_API int erofs_extract_configs(void) {
    return erofs_extract_configs_impl();
}

EROFS_API int erofs_extract_set_options(const erofs_extract_options* options) {
    return erofs_extract_set_options_impl(options);
}

EROFS_API const char* erofs_extract_get_error(void) {
    return erofs_extract_get_error_impl();
}

EROFS_API void erofs_extract_cleanup(void) {
    erofs_extract_cleanup_impl();
}