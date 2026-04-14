PROJECT_ROOT := ../../..

# Directories
SRC_DIR   := $(PROJECT_ROOT)/libraries/vf_file/src
INC_DIR   := $(PROJECT_ROOT)/libraries/vf_file/public-inc
BUILD_DIR := $(PROJECT_ROOT)/libraries/vf_file/build

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
VF_FILE_SRC := $(SRC_DIR)/vf-file.c
VF_FILE_OBJ := $(BUILD_DIR)/vf-file.o
VF_FILE_LIB := $(BUILD_DIR)/libvf-file.a

# ==============================================================================
# Rules
# ==============================================================================

.PHONY: all clean vf_common vf_logger

all: $(BUILD_DIR) vf_common vf_logger $(VF_FILE_LIB)

vf_common:
	@echo "[vf_file] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger: vf_common
	@echo "[vf_file] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

$(VF_FILE_OBJ): $(VF_FILE_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(VF_FILE_LIB): $(VF_FILE_OBJ)
	$(AR) rcs $@ $^
	@echo "[vf_file] libvf-file.a built successfully."

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "[vf_file] Cleaning vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	@echo "[vf_file] Cleaning vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(BUILD_DIR)/*.o $(VF_FILE_LIB)
	@echo "[vf_file] clean done."

