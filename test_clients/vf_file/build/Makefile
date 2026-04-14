PROJECT_ROOT := ../../..

# Directories
TEST_SRC_DIR   := $(PROJECT_ROOT)/test_clients/vf_file/src
TEST_BUILD_DIR := $(PROJECT_ROOT)/test_clients/vf_file/build

# Dependencies — vf_file
VF_FILE_INC_DIR := $(PROJECT_ROOT)/libraries/vf_file/public-inc
VF_FILE_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_file/build
VF_FILE_LIB     := $(VF_FILE_LIB_DIR)/libvf-file.a

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
          -I$(VF_FILE_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR) \
          -I$(VF_COMMON_INC_DIR)

# Files
TEST_SRC := $(TEST_SRC_DIR)/main.c
TEST_BIN := $(TEST_BUILD_DIR)/test_vf_file

# ==============================================================================
# Rules
# ==============================================================================

.PHONY: all clean run vf_common vf_logger vf_file

all: $(TEST_BUILD_DIR) vf_common vf_logger vf_file $(TEST_BIN)

vf_common:
	@echo "[test_vf_file] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger: vf_common
	@echo "[test_vf_file] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

vf_file: vf_logger
	@echo "[test_vf_file] Building vf_file..."
	$(MAKE) -C $(VF_FILE_LIB_DIR)

$(TEST_BIN): $(TEST_SRC) $(VF_FILE_LIB) $(VF_LOGGER_LIB) $(VF_COMMON_LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread
	@echo "[test_vf_file] test_vf_file built successfully."

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

run: all
	@echo "[test_vf_file] Running test_vf_file..."
	./$(TEST_BIN)

clean:
	@echo "[test_vf_file] Cleaning vf_file..."
	$(MAKE) -C $(VF_FILE_LIB_DIR) clean
	@echo "[test_vf_file] Cleaning vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	@echo "[test_vf_file] Cleaning vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(TEST_BIN)
	@echo "[test_vf_file] clean done."

