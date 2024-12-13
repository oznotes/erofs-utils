#ifdef EROFS_EXTRACT_EXPORTS

#include "erofs_extract_dll.h"
#include "ExtractOperation.h"
#include "ExtractHelper.h"
#include "Logging.h"
#include "Utils.h"
#include <erofs/print.h>
#include <erofs/config.h>
#include <erofs/internal.h>
#include <stdio.h>
#include <memory>
#include <string>
#include <stdarg.h>
#include <fcntl.h>

using namespace skkk;

extern struct erofs_configure cfg;
static struct erofs_sb_info g_sbi;

// Forward declarations
namespace skkk {
    int initErofsNodeByTargetPath(const std::string& targetPath);
}

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
static FILE* g_logFile = NULL;

static void log_message(const char* format, ...) {
    if (!g_logFile) {
        g_logFile = fopen("erofs_extract.log", "w");
        if (!g_logFile) return;
    }
    
    va_list args;
    va_start(args, format);
    vfprintf(g_logFile, format, args);
    va_end(args);
    
    // Flush immediately to ensure we see output even if crash occurs
    fflush(g_logFile);
}

static void init_default_config() {
    if (g_ctx && g_ctx->op) {
        g_ctx->op->check_decomp = true;
        g_ctx->op->useMultiThread = false;
        g_ctx->op->threadNum = 1;
        g_ctx->op->overwrite = false;
        g_ctx->op->isSilent = false;
        g_ctx->op->isExtractTarget = false;
        g_ctx->op->isExtractAllNode = false;
        cfg.c_dbg_lvl = EROFS_ERR;
    }
}

extern "C" {

int erofs_extract_init(const char* image_path) {
    log_message("Entering erofs_extract_init\n");
    if (!image_path) {
        log_message("Invalid image path\n");
        g_ctx->op->setLastError("Invalid image path (null)");
        return RET_EXTRACT_INIT_FAIL;
    }
    log_message("Image path: %s\n", image_path);

    if (g_ctx) {
        log_message("Already initialized\n");
        g_ctx->op->setLastError("Context already initialized");
        return RET_EXTRACT_INIT_FAIL;
    }

    log_message("Creating context...\n");
    g_ctx = new(std::nothrow) ExtractContext();
    if (!g_ctx) {
        log_message("Failed to create context\n");
        return RET_EXTRACT_INIT_FAIL;
    }

    log_message("Initializing EROFS config...\n");
    erofs_init_configure();
    cfg.c_dbg_lvl = EROFS_ERR;

    log_message("Setting default configuration...\n");
    init_default_config();

    log_message("Setting image path...\n");
    g_ctx->op->setImgPath(image_path);
    g_ctx->op->setLastError("");

    // Check if file exists first
    FILE* f = fopen(image_path, "rb");
    if (!f) {
        char errBuf[256];
        snprintf(errBuf, sizeof(errBuf), "Image file not found or cannot be opened: %s", image_path);
        g_ctx->op->setLastError(errBuf);
        delete g_ctx;
        g_ctx = nullptr;
        return RET_EXTRACT_INIT_FAIL;
    }
    fclose(f);

    log_message("Opening device...\n");
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

    log_message("Reading superblock...\n");
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

    log_message("Creating directories...\n");
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
    log_message("Initialization complete\n");
    return RET_EXTRACT_DONE;
}

int erofs_extract_set_outdir(const char* out_dir) {
    if (!out_dir) {
        return RET_EXTRACT_CONFIG_FAIL;
    }

    if (!g_ctx || !g_ctx->initialized) {
        log_message("DLL not initialized\n");
        return RET_EXTRACT_CONFIG_FAIL;
    }

    g_ctx->op->setOutDir(out_dir);
    int rc = g_ctx->op->initOutDir();
    if (!rc) {
        return RET_EXTRACT_CONFIG_FAIL;
    }
    return RET_EXTRACT_DONE;
}

int erofs_extract_path(const char* target_path, bool recursive) {
    if (!target_path) {
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    if (!g_ctx || !g_ctx->initialized) {
        log_message("DLL not initialized\n");
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

    err = initErofsNodeByTargetPath(std::string(target_path));
    if (err) {
        g_ctx->op->setLastError("Failed to initialize target node");
        return RET_EXTRACT_INIT_NODE_FAIL;
    }

    g_ctx->op->extractFsConfigAndSelinuxLabelAndFsOptions();
    g_ctx->op->extractErofsNode(g_ctx->op->isSilent);

    return RET_EXTRACT_DONE;
}

int erofs_extract_all(void) {
    if (!g_ctx || !g_ctx->initialized) {
        log_message("DLL not initialized\n");
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

int erofs_extract_set_options(const erofs_extract_options* options) {
    if (!options) {
        return RET_EXTRACT_CONFIG_FAIL;
    }

    if (!g_ctx || !g_ctx->initialized) {
        log_message("DLL not initialized\n");
        return RET_EXTRACT_CONFIG_FAIL;
    }

    g_ctx->op->overwrite = options->overwrite;
    g_ctx->op->isSilent = options->silent;

    if (options->debug_level >= 0) {
        cfg.c_dbg_lvl = options->debug_level;
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

const char* erofs_extract_get_error(void) {
    if (!g_ctx) {
        static const char* not_init = "DLL not initialized (context is null)";
        return not_init;
    }
    if (!g_ctx->initialized) {
        static const char* not_init_flag = "DLL not initialized (flag is false)";
        return not_init_flag;
    }
    const char* err = g_ctx->op->getLastError();
    return err && *err ? err : "Unknown error";
}

void erofs_extract_cleanup(void) {
    if (g_ctx) {
        erofs_dev_close(&g_sbi);
        erofs_blob_closeall(&g_sbi);
        erofs_exit_configure();
        ExtractOperation::erofsOperationExit();
        delete g_ctx;
        g_ctx = nullptr;
    }
    if (g_logFile) {
        fclose(g_logFile);
        g_logFile = NULL;
    }
}

// Add this simple test function to the DLL
int erofs_test_function(int input, int* output) {
    if (!g_logFile) {
        g_logFile = fopen("erofs_extract.log", "w");
    }
    
    if (g_logFile) {
        fprintf(g_logFile, "Test function called with input: %d\n", input);
        fflush(g_logFile);
    }

    if (!output) {
        return -1;
    }

    *output = input * 2;
    return 0;
}

// Add this extremely simple test function
int erofs_test_simple(void) {
    if (g_logFile) {
        fprintf(g_logFile, "Test simple function called\n");
        fflush(g_logFile);
    }
    return 42;  // Just return a constant value
}

} // extern "C"

#endif // EROFS_EXTRACT_EXPORTS