PROJECT_ROOT := ../../..

# Directories
SRC_DIR   := $(PROJECT_ROOT)/libraries/vf_framebuffer/src
INC_DIR   := $(PROJECT_ROOT)/libraries/vf_framebuffer/public
BUILD_DIR := $(PROJECT_ROOT)/libraries/vf_framebuffer/build

# Dependencies — vf_common
VF_COMMON_INC_DIR := $(PROJECT_ROOT)/libraries/vf_common/public
VF_COMMON_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_common/build
VF_COMMON_LIB     := $(VF_COMMON_LIB_DIR)/libvf-common.a

# Dependencies — vf_logger
VF_LOGGER_INC_DIR := $(PROJECT_ROOT)/libraries/vf_logger/public
VF_LOGGER_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_logger/build
VF_LOGGER_LIB     := $(VF_LOGGER_LIB_DIR)/libvf-logger.a

# Compiler
CC     := gcc
AR     := ar
CFLAGS := -Wall -Wextra -Werror \
          -I$(INC_DIR) \
          -I$(VF_COMMON_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR)

# Files
FB_SRC := $(SRC_DIR)/vf-framebuffer.c
FB_OBJ := $(BUILD_DIR)/vf-framebuffer.o
FB_LIB := $(BUILD_DIR)/libvf-framebuffer.a

# ==============================================================================
# Rules
# ==============================================================================

.PHONY: all clean vf_common vf_logger

all: $(BUILD_DIR) vf_common vf_logger $(FB_LIB)

vf_common:
	@echo "[vf_framebuffer] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger: vf_common
	@echo "[vf_framebuffer] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

$(FB_OBJ): $(FB_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(FB_LIB): $(FB_OBJ)
	$(AR) rcs $@ $^
	@echo "[vf_framebuffer] libvf-framebuffer.a built successfully."

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "[vf_framebuffer] Cleaning vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	@echo "[vf_framebuffer] Cleaning vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(BUILD_DIR)/*.o $(FB_LIB)
	@echo "[vf_framebuffer] clean done."

