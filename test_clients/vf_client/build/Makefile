PROJECT_ROOT := ../../..

# Directories
TEST_SRC_DIR   := $(PROJECT_ROOT)/test_clients/vf_client/src
TEST_BUILD_DIR := $(PROJECT_ROOT)/test_clients/vf_client/build

# Dependencies — vf_common
VF_COMMON_INC_DIR := $(PROJECT_ROOT)/libraries/vf_common/public
VF_COMMON_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_common/build
VF_COMMON_LIB     := $(VF_COMMON_LIB_DIR)/libvf-common.a

# Compiler
CC     := gcc
CFLAGS := -Wall -Wextra -Werror \
          -I$(VF_COMMON_INC_DIR)

# Files
TEST_SRC := $(TEST_SRC_DIR)/main.c
TEST_BIN := $(TEST_BUILD_DIR)/vf_client

# ==============================================================================
# Rules
# ==============================================================================
.PHONY: all clean run vf_common

all: $(TEST_BUILD_DIR) vf_common $(TEST_BIN)

vf_common:
	@echo "[vf_client] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

$(TEST_BIN): $(TEST_SRC) $(VF_COMMON_LIB)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "[vf_client] vf_client built successfully."

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

run: all
	@echo "[vf_client] Usage: ./$(TEST_BIN) --get_status | --get_stats | --stop | --start"

clean:
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(TEST_BIN)
	@echo "[vf_client] clean done."

