PROJECT_ROOT := ../../..

# Directories
SRC_DIR   := $(PROJECT_ROOT)/libraries/vf_logger/src
INC_DIR   := $(PROJECT_ROOT)/libraries/vf_logger/public
BUILD_DIR := $(PROJECT_ROOT)/libraries/vf_logger/build

# Dependencies
VF_COMMON_INC_DIR := $(PROJECT_ROOT)/libraries/vf_common/public
VF_COMMON_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_common/build
VF_COMMON_LIB     := $(VF_COMMON_LIB_DIR)/libvf-common.a

# Compiler
CC    := gcc
AR    := ar
CFLAGS := -Wall -Wextra -Werror \
          -I$(INC_DIR) \
          -I$(VF_COMMON_INC_DIR)

# Files
LOGGER_SRC := $(SRC_DIR)/vf-logger.c
LOGGER_OBJ := $(BUILD_DIR)/vf-logger.o
LOGGER_LIB := $(BUILD_DIR)/libvf-logger.a

# ==============================================================================
# Rules
# ==============================================================================

.PHONY: all clean vf_common

all: $(BUILD_DIR) vf_common $(LOGGER_LIB)

vf_common:
	@echo "[vf_logger] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

$(LOGGER_OBJ): $(LOGGER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(LOGGER_LIB): $(LOGGER_OBJ)
	$(AR) rcs $@ $^
	@echo "[vf_logger] libvf-logger.a built successfully."

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "[vf_logger] Cleaning vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(BUILD_DIR)/*.o $(LOGGER_LIB)
	@echo "[vf_logger] clean done."

