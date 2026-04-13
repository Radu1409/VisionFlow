PROJECT_ROOT := ../../..

# Directories
SRC_DIR   := $(PROJECT_ROOT)/test_clients/vf_service/src
BUILD_DIR := $(PROJECT_ROOT)/test_clients/vf_service/build

SERVICE_INC_DIR := $(PROJECT_ROOT)/service/public-inc
SERVICE_SRC_DIR := $(PROJECT_ROOT)/service/src/parser
SERVICE_BUILD_DIR := $(PROJECT_ROOT)/service/build

VF_COMMON_INC_DIR := $(PROJECT_ROOT)/libraries/vf_common/public
VF_COMMON_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_common/build
VF_COMMON_LIB     := $(VF_COMMON_LIB_DIR)/libvf-common.a

VF_LOGGER_INC_DIR := $(PROJECT_ROOT)/libraries/vf_logger/public
VF_LOGGER_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_logger/build
VF_LOGGER_LIB     := $(VF_LOGGER_LIB_DIR)/libvf-logger.a

# Compiler
CC := gcc
CFLAGS := -Wall -Wextra -Werror \
          -I$(SERVICE_INC_DIR) \
          -I$(SERVICE_SRC_DIR) \
          -I$(VF_COMMON_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR)

LDFLAGS := $(VF_COMMON_LIB) $(VF_LOGGER_LIB) -lpthread

# Files
TEST_SRC := $(SRC_DIR)/main.c
TEST_OBJ := $(BUILD_DIR)/main.o

PARSER_SRC := $(SERVICE_SRC_DIR)/vf-parser.c
PARSER_OBJ := $(BUILD_DIR)/vf-parser.o

TEST_BIN := $(BUILD_DIR)/test_vf_service

# ==============================================================================
# Rules
# ==============================================================================

.PHONY: all clean vf_common vf_logger

all: $(BUILD_DIR) vf_common vf_logger $(TEST_BIN)

vf_common:
	@echo "[test_vf_service] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger:
	@echo "[test_vf_service] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

$(TEST_OBJ): $(TEST_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(PARSER_OBJ): $(PARSER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST_OBJ) $(PARSER_OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "[test_vf_service] test_vf_service built successfully."

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "[test_vf_service] Cleaning dependencies..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(BUILD_DIR)/*.o $(TEST_BIN)
	@echo "[test_vf_service] clean done."

