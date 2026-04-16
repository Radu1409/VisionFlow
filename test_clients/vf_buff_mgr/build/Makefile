PROJECT_ROOT := ../../..

# Directories
TEST_SRC_DIR   := $(PROJECT_ROOT)/test_clients/vf_buff_mgr/src
TEST_BUILD_DIR := $(PROJECT_ROOT)/test_clients/vf_buff_mgr/build

# Dependencies — service
VF_SERVICE_INC_DIR := $(PROJECT_ROOT)/service/public-inc
VF_SERVICE_SRC_DIR := $(PROJECT_ROOT)/service/src/buffer-mgr
VF_SERVICE_BUILD_DIR := $(PROJECT_ROOT)/service/build

# Dependencies — vf_framebuffer
VF_FB_INC_DIR := $(PROJECT_ROOT)/libraries/vf_framebuffer/public
VF_FB_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_framebuffer/build
VF_FB_LIB     := $(VF_FB_LIB_DIR)/libvf-framebuffer.a

# Dependencies — vf_logger
VF_LOGGER_INC_DIR := $(PROJECT_ROOT)/libraries/vf_logger/public
VF_LOGGER_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_logger/build
VF_LOGGER_LIB     := $(VF_LOGGER_LIB_DIR)/libvf-logger.a

# Dependencies — vf_common
VF_COMMON_INC_DIR := $(PROJECT_ROOT)/libraries/vf_common/public
VF_COMMON_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_common/build
VF_COMMON_LIB     := $(VF_COMMON_LIB_DIR)/libvf-common.a

# Compiler
CC     := gcc
CFLAGS := -Wall -Wextra -Werror \
          -I$(VF_SERVICE_INC_DIR) \
          -I$(VF_SERVICE_SRC_DIR) \
          -I$(VF_FB_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR) \
          -I$(VF_COMMON_INC_DIR)

# Files
TEST_SRC         := $(TEST_SRC_DIR)/main.c
TEST_BIN         := $(TEST_BUILD_DIR)/test_vf_buff_mgr
VF_BUF_POOL_SRC  := $(VF_SERVICE_SRC_DIR)/vf-buff-pool.c
VF_BUF_QUEUE_SRC := $(VF_SERVICE_SRC_DIR)/vf-buff-queue.c

# ==============================================================================
# Rules
# ==============================================================================

.PHONY: all clean run vf_common vf_logger vf_framebuffer vf_service

all: $(TEST_BUILD_DIR) vf_common vf_logger vf_framebuffer vf_service $(TEST_BIN)

vf_common:
	@echo "[test_vf_buff_mgr] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger: vf_common
	@echo "[test_vf_buff_mgr] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

vf_framebuffer: vf_logger
	@echo "[test_vf_buff_mgr] Building vf_framebuffer..."
	$(MAKE) -C $(VF_FB_LIB_DIR)

vf_service: vf_framebuffer
	@echo "[test_vf_buff_mgr] Building service modules..."
	$(MAKE) -C $(VF_SERVICE_BUILD_DIR)

$(TEST_BIN): $(TEST_SRC) $(VF_BUF_POOL_SRC) $(VF_BUF_QUEUE_SRC) $(VF_FB_LIB) $(VF_LOGGER_LIB) $(VF_COMMON_LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread
	@echo "[test_vf_buff_mgr] test_vf_buff_mgr built successfully."

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

run: all
	@echo "[test_vf_buff_mgr] Running test_vf_buff_mgr..."
	./$(TEST_BIN)

clean:
	@echo "[test_vf_buff_mgr] Cleaning service build..."
	$(MAKE) -C $(VF_SERVICE_BUILD_DIR) clean
	@echo "[test_vf_buff_mgr] Cleaning vf_framebuffer..."
	$(MAKE) -C $(VF_FB_LIB_DIR) clean
	@echo "[test_vf_buff_mgr] Cleaning vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	@echo "[test_vf_buff_mgr] Cleaning vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(TEST_BIN)
	@echo "[test_vf_buff_mgr] clean done."

