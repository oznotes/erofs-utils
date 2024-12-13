#ifdef EROFS_EXTRACT_EXPORTS

#include "erofs_extract_dll.h"
#include "ExtractOperation.h"
#include "ExtractHelper.h"
#include <erofs/print.h>
#include <erofs/config.h>
#include <stdio.h>
#include <memory>

using namespace skkk;

extern struct erofs_configure cfg;
static struct erofs_sb_info g_sbi;

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

static ExtractContext* g_ctx = nullptr;

static void init_default_config() {
    if (g_ctx && g_ctx->op) {
        g_ctx->op->check_decomp = true;
        g_ctx->op->useMultiThread = false;
        g_ctx->op->threadNum = 1;
        g_ctx->op->overwrite = false;
        g_ctx->op->isSilent = false;
        g_ctx->op->isExtractTarget = false;
        g_ctx->op->isExtractAllNode = false;
        g_ctx->op->configPath = "";
        cfg.c_dbg_lvl = EROFS_ERR;
    }
}

extern "C" {

EROFS_API int __cdecl erofs_extract_init(const char* image_path) {
    printf("Entering erofs_extract_init\n");
    if (!image_path) {
        printf("Invalid image path\n");
        return RET_EXTRACT_INIT_FAIL;
    }

    if (g_ctx) {
        printf("Already initialized\n");
        return RET_EXTRACT_INIT_FAIL;
    }

    g_ctx = new(std::nothrow) ExtractContext();
    if (!g_ctx) {
        printf("Failed to create context\n");
        return RET_EXTRACT_INIT_FAIL;
    }

    // Initialize erofs config
    erofs_init_configure();
    cfg.c_dbg_lvl = EROFS_ERR;

    // Set default configuration
    init_default_config();

    g_ctx->op->setImgPath(image_path);
    g_ctx->op->setLastError("");

    // Open the device
    int err = erofs_dev_open(&g_sbi, image_path, O_RDONLY);
    if (err) {
        char errBuf[256];
        snprintf(errBuf, sizeof(errBuf), "Failed to open image file: %d", err);
        g_ctx->op->setLastError(errBuf);
        delete g_ctx;
        g_ctx = nullptr;
        return RET_EXTRACT_INIT_FAIL;
    }

    // Read superblock
    err = erofs_read_superblock(&g_sbi);
    if (err) {
        char errBuf[256];
        snprintf(errBuf, sizeof(errBuf), "Failed to read superblock: %d", err);
        g_ctx->op->setLastError(errBuf);
        erofs_dev_close(&g_sbi);
        delete g_ctx;
        g_ctx = nullptr;
        return RET_EXTRACT_INIT_FAIL;
    }

    // Create necessary directories
    err = g_ctx->op->createExtractConfigDir() & g_ctx->op->createExtractOutDir();
    if (err) {
        g_ctx->op->setLastError("Failed to create output directories");
        erofs_dev_close(&g_sbi);
        delete g_ctx;
        g_ctx = nullptr;
        return RET_EXTRACT_CREATE_DIR_FAIL;
    }

    g_ctx->initialized = true;
    return RET_EXTRACT_DONE;
}

EROFS_API int erofs_extract_set_outdir(const char* out_dir) {
    if (!out_dir) {
        return RET_EXTRACT_CONFIG_FAIL;
    }

    if (!g_ctx || !g_ctx->initialized) {
        printf("DLL not initialized\n");
        return RET_EXTRACT_CONFIG_FAIL;
    }

    g_ctx->op->setOutDir(out_dir);
    int rc = g_ctx->op->initOutDir();
    if (!rc) {
        return RET_EXTRACT_CONFIG_FAIL;
    }
    return RET_EXTRACT_DONE;
}

EROFS_API int erofs_extract_path(const char* target_path, bool recursive) {
    if (!target_path) {
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    if (!g_ctx || !g_ctx->initialized) {
        printf("DLL not initialized\n");
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    g_ctx->op->targetPath = target_path;
    g_ctx->op->check_decomp = true;
    g_ctx->op->isExtractTarget = true;
    g_ctx->op->targetConfigRecurse = recursive;

    int err = g_ctx->op->createExtractConfigDir() & g_ctx->op->createExtractOutDir();
    if (err) {
        g_ctx->op->setLastError("Failed to create output directories");
        return RET_EXTRACT_CREATE_DIR_FAIL;
    }

    err = initErofsNodeByTargetPath(string(target_path));
    if (err) {
        g_ctx->op->setLastError("Failed to initialize target node");
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    g_ctx->op->extractFsConfigAndSelinuxLabelAndFsOptions();
    g_ctx->op->extractErofsNode(g_ctx->op->isSilent);

    return RET_EXTRACT_DONE;
}

EROFS_API int erofs_extract_all(void) {
    if (!g_ctx || !g_ctx->initialized) {
        printf("DLL not initialized\n");
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    g_ctx->op->check_decomp = true;
    g_ctx->op->isExtractAllNode = true;

    int err = g_ctx->op->createExtractConfigDir() & g_ctx->op->createExtractOutDir();
    if (err) {
        return RET_EXTRACT_CREATE_DIR_FAIL;
    }

    err = g_ctx->op->initAllErofsNode();
    if (err) {
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    g_ctx->op->extractFsConfigAndSelinuxLabelAndFsOptions();
    g_ctx->op->useMultiThread ? 
        g_ctx->op->extractErofsNodeMultiThread(g_ctx->op->isSilent) :
        g_ctx->op->extractErofsNode(g_ctx->op->isSilent);

    return RET_EXTRACT_DONE;
}

EROFS_API int erofs_extract_set_options(const erofs_extract_options* options) {
    if (!options) {
        return RET_EXTRACT_CONFIG_FAIL;
    }

    if (!g_ctx || !g_ctx->initialized) {
        printf("DLL not initialized\n");
        return RET_EXTRACT_CONFIG_FAIL;
    }

    g_ctx->op->overwrite = options->overwrite;
    g_ctx->op->isSilent = options->silent;

    if (options->debug_level >= 0) {
        cfg.c_dbg_lvl = options->debug_level;
    }

    if (options->config_path) {
        g_ctx->op->configPath = options->config_path;
    }

    if (options->num_threads > 0) {
        g_ctx->op->useMultiThread = true;
        g_ctx->op->threadNum = options->num_threads;
        if (g_ctx->op->threadNum > g_ctx->op->limitHardwareConcurrency) {
            return RET_EXTRACT_THREAD_NUM_ERROR;
        }
    }

    return RET_EXTRACT_DONE;
}

EROFS_API const char* erofs_extract_get_error(void) {
    if (!g_ctx || !g_ctx->initialized) {
        return "DLL not initialized";
    }
    return g_ctx->op->getLastError();
}

EROFS_API void __cdecl erofs_extract_cleanup(void) {
    if (g_ctx) {
        erofs_dev_close(&g_sbi);
        erofs_blob_closeall(&g_sbi);
        erofs_exit_configure();
        ExtractOperation::erofsOperationExit();
        delete g_ctx;
        g_ctx = nullptr;
    }
}

} // extern "C"

#endif // EROFS_EXTRACT_EXPORTS