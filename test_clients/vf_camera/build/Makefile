PROJECT_ROOT := ../../..

# Directories
TEST_SRC_DIR   := $(PROJECT_ROOT)/test_clients/vf_camera/src
TEST_BUILD_DIR := $(PROJECT_ROOT)/test_clients/vf_camera/build
FRAMES_OUT_DIR := $(TEST_BUILD_DIR)/frames_out

# Dependencies — vf_camera
VF_CAMERA_INC_DIR := $(PROJECT_ROOT)/libraries/vf_camera/public
VF_CAMERA_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_camera/build
VF_CAMERA_LIB     := $(VF_CAMERA_LIB_DIR)/libvf-camera.a

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
          -I$(VF_CAMERA_INC_DIR) \
          -I$(VF_FB_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR) \
          -I$(VF_COMMON_INC_DIR)

# Files
TEST_SRC := $(TEST_SRC_DIR)/main.c
TEST_BIN := $(TEST_BUILD_DIR)/test_vf_camera

# ==============================================================================
# Rules
# ==============================================================================
.PHONY: all clean run vf_common vf_logger vf_framebuffer vf_camera

all: $(TEST_BUILD_DIR) $(FRAMES_OUT_DIR) vf_common vf_logger vf_framebuffer vf_camera $(TEST_BIN)

vf_common:
	@echo "[test_vf_camera] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger: vf_common
	@echo "[test_vf_camera] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

vf_framebuffer: vf_logger
	@echo "[test_vf_camera] Building vf_framebuffer..."
	$(MAKE) -C $(VF_FB_LIB_DIR)

vf_camera: vf_framebuffer
	@echo "[test_vf_camera] Building vf_camera..."
	$(MAKE) -C $(VF_CAMERA_LIB_DIR)

$(TEST_BIN): $(TEST_SRC) $(VF_CAMERA_LIB) $(VF_FB_LIB) $(VF_LOGGER_LIB) $(VF_COMMON_LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread
	@echo "[test_vf_camera] test_vf_camera built successfully."

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

$(FRAMES_OUT_DIR):
	mkdir -p $(FRAMES_OUT_DIR)

run: all
	@echo "[test_vf_camera] Running test_vf_camera..."
	cd $(TEST_BUILD_DIR) && ./test_vf_camera

clean:
	@echo "[test_vf_camera] Cleaning vf_camera..."
	$(MAKE) -C $(VF_CAMERA_LIB_DIR) clean
	@echo "[test_vf_camera] Cleaning vf_framebuffer..."
	$(MAKE) -C $(VF_FB_LIB_DIR) clean
	@echo "[test_vf_camera] Cleaning vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	@echo "[test_vf_camera] Cleaning vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(TEST_BIN)
	rm -rf $(FRAMES_OUT_DIR)
	@echo "[test_vf_camera] clean done."

