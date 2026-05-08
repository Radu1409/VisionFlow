PROJECT_ROOT := ../../..

# Directories
SRC_DIR   := $(PROJECT_ROOT)/libraries/vf_camera/src
INC_DIR   := $(PROJECT_ROOT)/libraries/vf_camera/public
BUILD_DIR := $(PROJECT_ROOT)/libraries/vf_camera/build

# Dependencies — vf_common
VF_COMMON_INC_DIR := $(PROJECT_ROOT)/libraries/vf_common/public
VF_COMMON_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_common/build
VF_COMMON_LIB     := $(VF_COMMON_LIB_DIR)/libvf-common.a

# Dependencies — vf_logger
VF_LOGGER_INC_DIR := $(PROJECT_ROOT)/libraries/vf_logger/public
VF_LOGGER_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_logger/build
VF_LOGGER_LIB     := $(VF_LOGGER_LIB_DIR)/libvf-logger.a

# Dependencies — vf_framebuffer
VF_FB_INC_DIR := $(PROJECT_ROOT)/libraries/vf_framebuffer/public
VF_FB_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_framebuffer/build
VF_FB_LIB     := $(VF_FB_LIB_DIR)/libvf-framebuffer.a

# Compiler
CC     := gcc
AR     := ar
CFLAGS := -Wall -Wextra -Werror \
          -I$(INC_DIR) \
          -I$(VF_COMMON_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR) \
          -I$(VF_FB_INC_DIR)

# Files
VF_CAMERA_SRC := $(SRC_DIR)/vf-camera.c
VF_CAMERA_OBJ := $(BUILD_DIR)/vf-camera.o
VF_CAMERA_LIB := $(BUILD_DIR)/libvf-camera.a

# ==============================================================================
# Rules
# ==============================================================================
.PHONY: all clean vf_common vf_logger vf_framebuffer

all: $(BUILD_DIR) vf_common vf_logger vf_framebuffer $(VF_CAMERA_LIB)

vf_common:
	@echo "[vf_camera] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger: vf_common
	@echo "[vf_camera] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

vf_framebuffer: vf_logger
	@echo "[vf_camera] Building vf_framebuffer..."
	$(MAKE) -C $(VF_FB_LIB_DIR)

$(VF_CAMERA_OBJ): $(VF_CAMERA_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(VF_CAMERA_LIB): $(VF_CAMERA_OBJ)
	$(AR) rcs $@ $^
	@echo "[vf_camera] libvf-camera.a built successfully."

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "[vf_camera] Cleaning vf_framebuffer..."
	$(MAKE) -C $(VF_FB_LIB_DIR) clean
	@echo "[vf_camera] Cleaning vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	@echo "[vf_camera] Cleaning vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(BUILD_DIR)/*.o $(VF_CAMERA_LIB)
	@echo "[vf_camera] clean done."

