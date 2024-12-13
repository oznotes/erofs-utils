#ifdef EROFS_EXTRACT_EXPORTS

#include "erofs_extract_dll.h"
#include "ExtractOperation.h"
#include <erofs/print.h>
#include <erofs/config.h>
#include <stdio.h>
#include <memory>

using namespace skkk;

// Internal context structure
struct ExtractContext {
    std::string last_error;
    ExtractOperation* op;
    bool initialized;

    ExtractContext() : initialized(false) {
        op = ExtractOperation::getInstance();
    }
    
    ~ExtractContext() {
        // Don't delete op since it's managed by getInstance()
    }
};

static void set_context_error(ExtractContext* ctx, const char* msg) {
    if (ctx) {
        ctx->last_error = msg ? msg : "Unknown error";
    }
}

extern "C" {

EROFS_API int __cdecl erofs_extract_init(const char* image_path) {
    printf("Entering erofs_extract_init\n");
    if (!image_path) {
        printf("Invalid image path\n");
        return RET_EXTRACT_INIT_FAIL;
    }

    ExtractContext* ctx = new ExtractContext();
    
    // Initialize erofs config
    erofs_init_configure();
    cfg.c_dbg_lvl = EROFS_ERR;

    ctx->op->setImgPath(image_path);
    ctx->op->setLastError("");

    // Open the device
    int err = erofs_dev_open(&g_sbi, image_path, O_RDONLY);
    if (err) {
        ctx->op->setLastError("Failed to open image file");
        return RET_EXTRACT_INIT_FAIL;
    }

    // Read superblock
    err = erofs_read_superblock(&g_sbi);
    if (err) {
        ctx->op->setLastError("Failed to read superblock");
        return RET_EXTRACT_INIT_FAIL;
    }

    ctx->initialized = true;
    return RET_EXTRACT_DONE;
}

EROFS_API int erofs_extract_set_outdir(const char* out_dir) {
    if (!out_dir) {
        return RET_EXTRACT_CONFIG_FAIL;
    }

    auto ctx = ExtractOperation::getInstance();
    ctx->setOutDir(out_dir);
    int rc = ctx->initOutDir();
    if (!rc) {
        return RET_EXTRACT_CONFIG_FAIL;
    }
    return RET_EXTRACT_DONE;
}

EROFS_API int erofs_extract_path(const char* target_path, bool recursive) {
    if (!target_path) {
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    auto ctx = ExtractOperation::getInstance();
    ctx->targetPath = target_path;
    ctx->check_decomp = true;
    ctx->isExtractTarget = true;
    ctx->targetConfigRecurse = recursive;

    int err = ctx->createExtractConfigDir() & ctx->createExtractOutDir();
    if (err) {
        ctx->setLastError("Failed to create output directories");
        return RET_EXTRACT_CREATE_DIR_FAIL;
    }

    err = initErofsNodeByTargetPath(target_path);
    if (err) {
        ctx->setLastError("Failed to initialize target node");
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    ctx->extractFsConfigAndSelinuxLabelAndFsOptions();
    ctx->extractErofsNode(ctx->isSilent);

    return RET_EXTRACT_DONE;
}

EROFS_API int erofs_extract_all(void) {
    auto ctx = ExtractOperation::getInstance();
    ctx->check_decomp = true;
    ctx->isExtractAllNode = true;

    int err = ctx->createExtractConfigDir() & ctx->createExtractOutDir();
    if (err) {
        return RET_EXTRACT_CREATE_DIR_FAIL;
    }

    err = ctx->initAllErofsNode();
    if (err) {
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    ctx->extractFsConfigAndSelinuxLabelAndFsOptions();
    ctx->useMultiThread ? 
        ctx->extractErofsNodeMultiThread(ctx->isSilent) :
        ctx->extractErofsNode(ctx->isSilent);

    return RET_EXTRACT_DONE;
}

EROFS_API int erofs_extract_set_options(const erofs_extract_options* options) {
    if (!options) {
        return RET_EXTRACT_CONFIG_FAIL;
    }

    auto ctx = ExtractOperation::getInstance();
    ctx->overwrite = options->overwrite;
    ctx->preserve_owner = options->preserve_owner;
    ctx->preserve_perms = options->preserve_perms;
    ctx->isSilent = options->silent;

    if (options->num_threads > 0) {
        ctx->useMultiThread = true;
        ctx->threadNum = options->num_threads;
        if (ctx->threadNum > ctx->limitHardwareConcurrency) {
            return RET_EXTRACT_THREAD_NUM_ERROR;
        }
    }

    return RET_EXTRACT_DONE;
}

EROFS_API const char* erofs_extract_get_error(void) {
    auto ctx = ExtractOperation::getInstance();
    return ctx->getLastError().c_str();
}

EROFS_API void erofs_extract_cleanup(void) {
    erofs_dev_close(&g_sbi);
    erofs_blob_closeall(&g_sbi);
    erofs_exit_configure();
    ExtractOperation::erofsOperationExit();
}

} // extern "C"

#endif // EROFS_EXTRACT_EXPORTS