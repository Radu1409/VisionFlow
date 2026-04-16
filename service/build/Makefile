PROJECT_ROOT := ../..

# Directories
SERVICE_SRC_DIR := $(PROJECT_ROOT)/service/src
SERVICE_INC_DIR := $(PROJECT_ROOT)/service/public-inc
BUILD_DIR       := $(PROJECT_ROOT)/service/build

# Parser sources
PARSER_SRC_DIR  := $(SERVICE_SRC_DIR)/parser
PARSER_SRC      := $(PARSER_SRC_DIR)/vf-parser.c
PARSER_OBJ      := $(BUILD_DIR)/vf-parser.o

# Buffer mgr sources
BUFFER_MGR_SRC_DIR  := $(SERVICE_SRC_DIR)/buffer-mgr
BUFFER_MGR_SRC      := $(BUFFER_MGR_SRC_DIR)/vf-buff-pool.c
BUFFER_MGR_SRC      := $(BUFFER_MGR_SRC_DIR)/vf-buff-queue.c
BUFFER_MGR_OBJ      := $(BUILD_DIR)/vf-buff-pool.o
BUFFER_MGR_OBJ      := $(BUILD_DIR)/vf-buff-queue.o

# Dependencies
VF_COMMON_INC_DIR := $(PROJECT_ROOT)/libraries/vf_common/public
VF_COMMON_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_common/build
VF_COMMON_LIB     := $(VF_COMMON_LIB_DIR)/libvf-common.a

VF_LOGGER_INC_DIR := $(PROJECT_ROOT)/libraries/vf_logger/public
VF_LOGGER_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_logger/build
VF_LOGGER_LIB     := $(VF_LOGGER_LIB_DIR)/libvf-logger.a

VF_FRAMEBUFFER_INC_DIR := $(PROJECT_ROOT)/libraries/vf_framebuffer/public
VF_FRAMEBUFFER_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_framebuffer/build
VF_FRAMEBUFFER_LIB     := $(VF_FRAMEBUFFER_LIB_DIR)/libvf-framebuffer.a

# Compiler
CC := gcc
CFLAGS := -Wall -Wextra -Werror \
          -I$(SERVICE_INC_DIR) \
          -I$(PARSER_SRC_DIR) \
          -I$(BUFFER_MGR_SRC_DIR) \
          -I$(VF_COMMON_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR) \
          -I$(VF_FRAMEBUFFER_INC_DIR)

LDFLAGS := $(VF_COMMON_LIB) $(VF_LOGGER_LIB)

.PHONY: all clean vf_common vf_logger vf_framebuffer

all: $(BUILD_DIR) vf_common vf_logger vf_framebuffer $(PARSER_OBJ) $(BUFFER_MGR_OBJ)

vf_common:
	@echo "[service] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger:
	@echo "[service] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

vf_framebuffer:
	@echo "[service] Building vf_framebuffer..."
	$(MAKE) -C $(VF_FRAMEBUFFER_LIB_DIR)

$(PARSER_OBJ): $(PARSER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUFFER_MGR_OBJ): $(BUFFER_MGR_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "[service] Cleaning dependencies..."
	$(MAKE) -C $(VF_FRAMEBUFFER_LIB_DIR) clean
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(BUILD_DIR)/*.o
	@echo "[service] clean done."

