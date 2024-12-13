#ifdef EROFS_EXTRACT_EXPORTS

#include "erofs_extract_dll.h"
#include "erofs_extract_impl.h"
#include "ExtractOperation.h"
#include <stdio.h>
#include <memory>

using namespace skkk;

// Internal context structure
struct ExtractContext {
    std::string last_error;
    std::unique_ptr<ExtractOperation> op;
    bool initialized;

    ExtractContext() : initialized(false) {
        op = std::make_unique<ExtractOperation>();
    }
};

extern "C" {

static void set_context_error(ExtractContext* ctx, const char* msg) {
    if (ctx) {
        ctx->last_error = msg ? msg : "Unknown error";
    }
}

EROFS_API int __cdecl erofs_extract_init(const char* image_path) {
    printf("Entering erofs_extract_init\n");
    if (!image_path) {
        printf("Invalid image path\n");
        return RET_EXTRACT_INIT_FAIL;
    }

    try {
        auto ctx = new ExtractContext();
        
        // Initialize erofs config
        erofs_init_configure();
        cfg.c_dbg_lvl = EROFS_ERR;

        ctx->op->setImgPath(image_path);

        // Open the device
        int err = erofs_dev_open(&g_sbi, image_path, O_RDONLY);
        if (err) {
            set_context_error(ctx, "Failed to open image file");
            return RET_EXTRACT_INIT_FAIL;
        }

        // Read superblock
        err = erofs_read_superblock(&g_sbi);
        if (err) {
            set_context_error(ctx, "Failed to read superblock");
            return RET_EXTRACT_INIT_FAIL;
        }

        ctx->initialized = true;
        return RET_EXTRACT_DONE;
    }
    catch (const std::exception& e) {
        printf("Exception in init: %s\n", e.what());
        return RET_EXTRACT_INIT_FAIL;
    }
}

EROFS_API int erofs_extract_set_outdir(const char* out_dir) {
    if (!out_dir) {
        return RET_EXTRACT_CONFIG_FAIL;
    }

    try {
        auto ctx = ExtractOperation::getInstance();
        ctx->setOutDir(out_dir);
        int rc = ctx->initOutDir();
        if (!rc) {
            return RET_EXTRACT_CONFIG_FAIL;
        }
        return RET_EXTRACT_DONE;
    }
    catch (const std::exception& e) {
        printf("Exception in set_outdir: %s\n", e.what());
        return RET_EXTRACT_CONFIG_FAIL;
    }
}

EROFS_API int erofs_extract_path(const char* target_path, bool recursive) {
    if (!target_path) {
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    try {
        auto ctx = ExtractOperation::getInstance();
        ctx->targetPath = target_path;
        ctx->check_decomp = true;
        ctx->isExtractTarget = true;

        int err = ctx->createExtractConfigDir() & ctx->createExtractOutDir();
        if (err) {
            return RET_EXTRACT_CREATE_DIR_FAIL;
        }

        err = initErofsNodeByTargetPath(target_path);
        if (err) {
            return RET_EXTRACT_INIT_NODE_FAIL;
        }

        ctx->extractFsConfigAndSelinuxLabelAndFsOptions();
        ctx->extractErofsNode(ctx->isSilent);

        return RET_EXTRACT_DONE;
    }
    catch (const std::exception& e) {
        printf("Exception in extract_path: %s\n", e.what());
        return RET_EXTRACT_INIT_NODE_FAIL;
    }
}

EROFS_API int erofs_extract_all(void) {
    try {
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
    catch (const std::exception& e) {
        printf("Exception in extract_all: %s\n", e.what());
        return RET_EXTRACT_INIT_NODE_FAIL;
    }
}

EROFS_API int erofs_extract_set_options(const erofs_extract_options* options) {
    if (!options) {
        return RET_EXTRACT_CONFIG_FAIL;
    }

    try {
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
    catch (const std::exception& e) {
        printf("Exception in set_options: %s\n", e.what());
        return RET_EXTRACT_CONFIG_FAIL;
    }
}

EROFS_API const char* erofs_extract_get_error(void) {
    try {
        auto ctx = ExtractOperation::getInstance();
        return ctx->getLastError().c_str();
    }
    catch (...) {
        return "Unknown error occurred";
    }
}

EROFS_API void erofs_extract_cleanup(void) {
    try {
        erofs_dev_close(&g_sbi);
        erofs_blob_closeall(&g_sbi);
        erofs_exit_configure();
        ExtractOperation::erofsOperationExit();
    }
    catch (const std::exception& e) {
        printf("Exception in cleanup: %s\n", e.what());
    }
}

} // extern "C"

#endif // EROFS_EXTRACT_EXPORTS