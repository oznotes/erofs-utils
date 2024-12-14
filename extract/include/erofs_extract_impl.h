#ifndef EROFS_EXTRACT_IMPL_H
#define EROFS_EXTRACT_IMPL_H

#include "erofs_extract_dll.h"

// Internal implementation functions - no exports needed
int erofs_extract_init_impl(const char* image_path);
int erofs_extract_set_outdir_impl(const char* out_dir);
int erofs_extract_path_impl(const char* target_path, bool recursive);
int erofs_extract_all_impl(void);
int erofs_extract_configs_impl(void);
int erofs_extract_set_options_impl(const erofs_extract_options* options);
const char* erofs_extract_get_error_impl(void);
void erofs_extract_cleanup_impl(void);

#endif // EROFS_EXTRACT_IMPL_H