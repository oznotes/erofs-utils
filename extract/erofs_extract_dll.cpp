#include "erofs_extract_dll.h"
#include "ExtractOperation.h"
#include "ExtractState.h"
#include "ExtractHelper.h"
#include <erofs/internal.h>
#include <thread>
#include <string>

#define EROFS_EXTRACT_EXPORTS

using namespace skkk;

static std::string last_error;

static void set_error(const char* msg) {
    last_error = msg;
}

EROFS_API int erofs_extract_init(const char* image_path) {
    try {
        // Initialize erofs config
        erofs_init_configure();
        cfg.c_dbg_lvl = EROFS_ERR;

        // Set image path
        eo->setImgPath(image_path);

        // Open the device
        int err = erofs_dev_open(&g_sbi, image_path, O_RDONLY);
        if (err) {
            set_error("Failed to open image file");
            return RET_EXTRACT_INIT_FAIL;
        }

        // Read superblock
        err = erofs_read_superblock(&g_sbi);
        if (err) {
            set_error("Failed to read superblock");
            return RET_EXTRACT_INIT_FAIL;
        }

        return RET_EXTRACT_DONE;
    } catch (const std::exception& e) {
        set_error(e.what());
        return RET_EXTRACT_INIT_FAIL;
    }
}

EROFS_API int erofs_extract_set_outdir(const char* out_dir) {
    try {
        eo->setOutDir(out_dir);
        int rc = eo->initOutDir();
        if (!rc) {
            set_error("Failed to initialize output directory");
            return RET_EXTRACT_CONFIG_FAIL;
        }
        return RET_EXTRACT_DONE;
    } catch (const std::exception& e) {
        set_error(e.what());
        return RET_EXTRACT_CONFIG_FAIL;
    }
}

EROFS_API int erofs_extract_path(const char* target_path, bool recursive) {
    try {
        eo->targetPath = target_path;
        eo->check_decomp = true;
        eo->isExtractTarget = true;

        int err = eo->createExtractConfigDir() & eo->createExtractOutDir();
        if (err) {
            set_error("Failed to create output directories");
            return RET_EXTRACT_CREATE_DIR_FAIL;
        }

        err = doInitNode(target_path, recursive);
        if (err) {
            set_error("Failed to initialize target node");
            return RET_EXTRACT_INIT_NODE_FAIL;
        }

        eo->extractFsConfigAndSelinuxLabelAndFsOptions();
        eo->extractErofsNode(eo->isSilent);

        return RET_EXTRACT_DONE;
    } catch (const std::exception& e) {
        set_error(e.what());
        return RET_EXTRACT_FAIL_EXIT;
    }
}

EROFS_API int erofs_extract_all(void) {
    try {
        eo->check_decomp = true;
        eo->isExtractAllNode = true;

        int err = eo->createExtractConfigDir() & eo->createExtractOutDir();
        if (err) {
            set_error("Failed to create output directories");
            return RET_EXTRACT_CREATE_DIR_FAIL;
        }

        err = eo->initAllErofsNode();
        if (err) {
            set_error("Failed to initialize nodes");
            return RET_EXTRACT_INIT_NODE_FAIL;
        }

        eo->extractFsConfigAndSelinuxLabelAndFsOptions();
        eo->useMultiThread ? 
            eo->extractErofsNodeMultiThread(eo->isSilent) : 
            eo->extractErofsNode(eo->isSilent);

        return RET_EXTRACT_DONE;
    } catch (const std::exception& e) {
        set_error(e.what());
        return RET_EXTRACT_FAIL_EXIT;
    }
}

EROFS_API int erofs_extract_configs(void) {
    try {
        eo->extractOnlyConfAndSeLabel = true;
        
        int err = eo->createExtractConfigDir();
        if (err) {
            set_error("Failed to create config directory");
            return RET_EXTRACT_CREATE_DIR_FAIL;
        }

        err = eo->initAllErofsNode();
        if (err) {
            set_error("Failed to initialize nodes");
            return RET_EXTRACT_INIT_NODE_FAIL;
        }

        eo->extractFsConfigAndSelinuxLabelAndFsOptions();
        return RET_EXTRACT_DONE;
    } catch (const std::exception& e) {
        set_error(e.what());
        return RET_EXTRACT_FAIL_EXIT;
    }
}

EROFS_API int erofs_extract_set_options(const erofs_extract_options* options) {
    try {
        if (!options) {
            set_error("Invalid options pointer");
            return RET_EXTRACT_CONFIG_FAIL;
        }

        eo->overwrite = options->overwrite;
        eo->preserve_owner = options->preserve_owner;
        eo->preserve_perms = options->preserve_perms;
        eo->isSilent = options->silent;

        if (options->num_threads > 0) {
            eo->useMultiThread = true;
            eo->threadNum = options->num_threads;
            if (eo->threadNum > eo->limitHardwareConcurrency) {
                set_error("Thread count exceeds limit");
                return RET_EXTRACT_THREAD_NUM_ERROR;
            }
        }

        return RET_EXTRACT_DONE;
    } catch (const std::exception& e) {
        set_error(e.what());
        return RET_EXTRACT_CONFIG_FAIL;
    }
}

EROFS_API const char* erofs_extract_get_error(void) {
    return last_error.c_str();
}

EROFS_API void erofs_extract_cleanup(void) {
    erofs_dev_close(&g_sbi);
    erofs_blob_closeall(&g_sbi);
    erofs_exit_configure();
    ExtractOperation::erofsOperationExit();
}