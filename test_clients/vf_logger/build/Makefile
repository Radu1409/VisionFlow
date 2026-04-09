PROJECT_ROOT := ../../..

# Directories
TEST_SRC_DIR   := $(PROJECT_ROOT)/test_clients/vf_logger/src
TEST_BUILD_DIR := $(PROJECT_ROOT)/test_clients/vf_logger/build

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
          -I$(VF_LOGGER_INC_DIR) \
          -I$(VF_COMMON_INC_DIR)

# Files
TEST_SRC := $(TEST_SRC_DIR)/main.c
TEST_BIN := $(TEST_BUILD_DIR)/test_vf_logger

# ==============================================================================
# Rules
# ==============================================================================

.PHONY: all clean run vf_common vf_logger

all: $(TEST_BUILD_DIR) vf_common vf_logger $(TEST_BIN)

vf_common:
	@echo "[test_vf_logger] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger: vf_common
	@echo "[test_vf_logger] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

$(TEST_BIN): $(TEST_SRC) $(VF_LOGGER_LIB) $(VF_COMMON_LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread
	@echo "[test_vf_logger] test_vf_logger built successfully."

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

run: all
	@echo "[test_vf_logger] Running test_vf_logger..."
	./$(TEST_BIN)

clean:
	@echo "[test_vf_logger] Cleaning vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	@echo "[test_vf_logger] Cleaning vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	rm -f $(TEST_BIN)
	@echo "[test_vf_logger] clean done."

