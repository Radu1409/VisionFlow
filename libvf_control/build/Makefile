PROJECT_ROOT := ../..

# Directories
SRC_DIR   := $(PROJECT_ROOT)/libvf_control/src
INC_DIR   := $(PROJECT_ROOT)/libvf_control/public
BUILD_DIR := $(PROJECT_ROOT)/libvf_control/build

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

# Dependencies — vf_camera
VF_CAMERA_INC_DIR := $(PROJECT_ROOT)/libraries/vf_camera/public

# Dependencies — vf_file
VF_FILE_INC_DIR := $(PROJECT_ROOT)/libraries/vf_file/public-inc

# Dependencies — vf_conversion
VF_CONVERSION_INC_DIR := $(PROJECT_ROOT)/libraries/vf_conversion/public

# Dependencies — service units
VF_SERVICE_INC_DIR      := $(PROJECT_ROOT)/service/public-inc
VF_SERVICE_UNITS_INC    := $(PROJECT_ROOT)/service/src/units/camera
VF_SERVICE_UNITS_INC    += $(PROJECT_ROOT)/service/src/units/stream-provider
VF_SERVICE_UNITS_INC    += $(PROJECT_ROOT)/service/src/units/conversion
VF_SERVICE_UNITS_INC    += $(PROJECT_ROOT)/service/src/units/file
VF_SERVICE_BUFFER_INC   := $(PROJECT_ROOT)/service/src/buffer-mgr
VF_SERVICE_PIPELINE_INC := $(PROJECT_ROOT)/service/src/core

# Compiler
CC := gcc
AR := ar

CFLAGS := -Wall -Wextra -Werror \
          -I$(INC_DIR) \
          -I$(VF_COMMON_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR) \
          -I$(VF_FB_INC_DIR) \
          -I$(VF_CAMERA_INC_DIR) \
          -I$(VF_FILE_INC_DIR) \
          -I$(VF_CONVERSION_INC_DIR) \
          -I$(VF_SERVICE_INC_DIR) \
          -I$(PROJECT_ROOT)/service/src/units/camera \
          -I$(PROJECT_ROOT)/service/src/units/stream-provider \
          -I$(PROJECT_ROOT)/service/src/units/conversion \
          -I$(PROJECT_ROOT)/service/src/units/file \
          -I$(PROJECT_ROOT)/service/src/buffer-mgr \
          -I$(PROJECT_ROOT)/service/src/core \
          -I$(PROJECT_ROOT)/service/src/notifier \
          -I$(PROJECT_ROOT)/service/src/parser

# Files
VF_CONTROL_SRC := $(SRC_DIR)/vf-control-server.c
VF_CONTROL_SRC += $(SRC_DIR)/vf-control-handler.c
VF_CONTROL_OBJ := $(BUILD_DIR)/vf-control-server.o
VF_CONTROL_OBJ += $(BUILD_DIR)/vf-control-handler.o
VF_CONTROL_LIB := $(BUILD_DIR)/libvf-control.a

# ==============================================================================
# Rules
# ==============================================================================
.PHONY: all clean vf_common vf_logger

all: $(BUILD_DIR) vf_common vf_logger $(VF_CONTROL_LIB)

vf_common:
	@echo "[vf_control] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger: vf_common
	@echo "[vf_control] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

$(BUILD_DIR)/vf-control-server.o: $(SRC_DIR)/vf-control-server.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vf-control-handler.o: $(SRC_DIR)/vf-control-handler.c
	$(CC) $(CFLAGS) -c $< -o $@

$(VF_CONTROL_LIB): $(VF_CONTROL_OBJ)
	$(AR) rcs $@ $^
	@echo "[vf_control] libvf-control.a built successfully."

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "[vf_control] Cleaning vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	@echo "[vf_control] Cleaning vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(BUILD_DIR)/*.o $(VF_CONTROL_LIB)
	@echo "[vf_control] clean done."

