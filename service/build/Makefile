PROJECT_ROOT := ../..

# Directories
SERVICE_SRC_DIR := $(PROJECT_ROOT)/service/src
SERVICE_INC_DIR := $(PROJECT_ROOT)/service/public-inc
BUILD_DIR       := $(PROJECT_ROOT)/service/build

# Parser sources
PARSER_SRC_DIR  := $(SERVICE_SRC_DIR)/parser
PARSER_SRC      := $(PARSER_SRC_DIR)/vf-parser.c
PARSER_OBJ      := $(BUILD_DIR)/vf-parser.o

# Dependencies
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
          -I$(PARSER_SRC_DIR) \
          -I$(VF_COMMON_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR)

LDFLAGS := $(VF_COMMON_LIB) $(VF_LOGGER_LIB)

.PHONY: all clean vf_common vf_logger

all: $(BUILD_DIR) vf_common vf_logger $(PARSER_OBJ)

vf_common:
	@echo "[service] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger:
	@echo "[service] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

$(PARSER_OBJ): $(PARSER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "[service] Cleaning dependencies..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(BUILD_DIR)/*.o
	@echo "[service] clean done."

