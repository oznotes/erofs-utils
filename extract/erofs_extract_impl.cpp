#include "erofs_extract_impl.h"
#include "erofs_extract_dll.h"
#include "ExtractOperation.h"
#include "ExtractState.h"
#include "ExtractHelper.h"
#include <erofs/print.h>
#include <erofs/internal.h>
#include <erofs/io.h>
#include <erofs/config.h>
#include <thread>
#include <string>
#include <stdio.h>

using namespace skkk;

static std::string last_error;

static void set_error(const char* msg) {
    last_error = msg;
}

int erofs_extract_init_impl(const char* image_path) {
    printf("Entering erofs_extract_init_impl\n");
    if (!image_path) {
        printf("Invalid image path in impl\n");
        return RET_EXTRACT_INIT_FAIL;
    }
    printf("Initializing erofs config\n");

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
}

int erofs_extract_set_outdir_impl(const char* out_dir) {
    if (!out_dir) {
        set_error("Invalid output directory");
        return RET_EXTRACT_CONFIG_FAIL;
    }

    eo->setOutDir(out_dir);
    int rc = eo->initOutDir();
    if (!rc) {
        set_error("Failed to initialize output directory");
        return RET_EXTRACT_CONFIG_FAIL;
    }
    return RET_EXTRACT_DONE;
}

int erofs_extract_path_impl(const char* target_path, bool recursive) {
    if (!target_path) {
        set_error("Invalid target path");
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    eo->targetPath = target_path;
    eo->check_decomp = true;
    eo->isExtractTarget = true;

    int err = eo->createExtractConfigDir() & eo->createExtractOutDir();
    if (err) {
        set_error("Failed to create output directories");
        return RET_EXTRACT_CREATE_DIR_FAIL;
    }

    err = initErofsNodeByTargetPath(target_path);
    if (err) {
        set_error("Failed to initialize target node");
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    eo->extractFsConfigAndSelinuxLabelAndFsOptions();
    eo->extractErofsNode(eo->isSilent);

    return RET_EXTRACT_DONE;
}

int erofs_extract_all_impl(void) {
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
}

int erofs_extract_configs_impl(void) {
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
}

int erofs_extract_set_options_impl(const erofs_extract_options* options) {
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
}

const char* erofs_extract_get_error_impl(void) {
    return last_error.c_str();
}

void erofs_extract_cleanup_impl(void) {
    erofs_dev_close(&g_sbi);
    erofs_blob_closeall(&g_sbi);
    erofs_exit_configure();
    ExtractOperation::erofsOperationExit();
}