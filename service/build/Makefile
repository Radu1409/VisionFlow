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
BUFFER_MGR_SRC      += $(BUFFER_MGR_SRC_DIR)/vf-buff-queue.c
BUFFER_MGR_OBJ      := $(BUILD_DIR)/vf-buff-pool.o
BUFFER_MGR_OBJ      += $(BUILD_DIR)/vf-buff-queue.o

# Core sources
CORE_SRC_DIR := $(SERVICE_SRC_DIR)/core
CORE_SRC     := $(CORE_SRC_DIR)/vf-processing-unit.c
CORE_SRC     += $(CORE_SRC_DIR)/vf-pipeline-mgr.c
CORE_SRC     += $(CORE_SRC_DIR)/vf-core.c
CORE_OBJ     := $(BUILD_DIR)/vf-processing-unit.o
CORE_OBJ     += $(BUILD_DIR)/vf-pipeline-mgr.o
CORE_OBJ     += $(BUILD_DIR)/vf-core.o

# Main
MAIN_SRC := $(SERVICE_SRC_DIR)/main.c
MAIN_OBJ := $(BUILD_DIR)/main.o

# Units sources
FILE_UNIT_SRC_DIR := $(SERVICE_SRC_DIR)/units/file
FILE_UNIT_SRC     := $(FILE_UNIT_SRC_DIR)/vf-file-unit.c
FILE_UNIT_OBJ     := $(BUILD_DIR)/vf-file-unit.o

CONVERSION_UNIT_SRC_DIR := $(SERVICE_SRC_DIR)/units/conversion
CONVERSION_UNIT_SRC := $(CONVERSION_UNIT_SRC_DIR)/vf-conversion-unit.c
CONVERSION_UNIT_OBJ := $(BUILD_DIR)/vf-conversion-unit.o

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

VF_FILE_INC_DIR := $(PROJECT_ROOT)/libraries/vf_file/public-inc
VF_FILE_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_file/build
VF_FILE_LIB     := $(VF_FILE_LIB_DIR)/libvf-file.a

VF_CONVERSION_INC_DIR := $(PROJECT_ROOT)/libraries/vf_conversion/public
VF_CONVERSION_LIB_DIR := $(PROJECT_ROOT)/libraries/vf_conversion/build
VF_CONVERSION_LIB     := $(VF_CONVERSION_LIB_DIR)/libvf-conversion.a

# Output binary
TARGET := $(BUILD_DIR)/vf_service

# Compiler
CC := gcc
CFLAGS := -Wall -Wextra -Werror \
          -I$(SERVICE_INC_DIR) \
          -I$(PARSER_SRC_DIR) \
          -I$(BUFFER_MGR_SRC_DIR) \
          -I$(CORE_SRC_DIR) \
          -I$(FILE_UNIT_SRC_DIR) \
          -I$(CONVERSION_UNIT_SRC_DIR) \
          -I$(VF_COMMON_INC_DIR) \
          -I$(VF_LOGGER_INC_DIR) \
          -I$(VF_FRAMEBUFFER_INC_DIR) \
          -I$(VF_FILE_INC_DIR) \
          -I$(VF_CONVERSION_INC_DIR)

LDFLAGS := $(VF_COMMON_LIB) $(VF_LOGGER_LIB)

.PHONY: all clean run vf_common vf_logger vf_framebuffer vf_file vf_conversion

all: $(BUILD_DIR) vf_common vf_logger vf_framebuffer vf_file vf_conversion \
     $(PARSER_OBJ) $(BUFFER_MGR_OBJ) $(CORE_OBJ) \
     $(FILE_UNIT_OBJ) $(CONVERSION_UNIT_OBJ) \
	 $(MAIN_OBJ) $(TARGET)

vf_common:
	@echo "[service] Building vf_common..."
	$(MAKE) -C $(VF_COMMON_LIB_DIR)

vf_logger:
	@echo "[service] Building vf_logger..."
	$(MAKE) -C $(VF_LOGGER_LIB_DIR)

vf_framebuffer:
	@echo "[service] Building vf_framebuffer..."
	$(MAKE) -C $(VF_FRAMEBUFFER_LIB_DIR)

vf_file:
	@echo "[service] Building vf_file..."
	$(MAKE) -C $(VF_FILE_LIB_DIR)

vf_conversion:
	@echo "[service] Building vf_conversion..."
	$(MAKE) -C $(VF_CONVERSION_LIB_DIR)

$(PARSER_OBJ): $(PARSER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Buffer mgr — reguli separate
$(BUILD_DIR)/vf-buff-pool.o: $(BUFFER_MGR_SRC_DIR)/vf-buff-pool.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vf-buff-queue.o: $(BUFFER_MGR_SRC_DIR)/vf-buff-queue.c
	$(CC) $(CFLAGS) -c $< -o $@

# Core — reguli separate
$(BUILD_DIR)/vf-processing-unit.o: $(CORE_SRC_DIR)/vf-processing-unit.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vf-pipeline-mgr.o: $(CORE_SRC_DIR)/vf-pipeline-mgr.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vf-core.o: $(CORE_SRC_DIR)/vf-core.c
	$(CC) $(CFLAGS) -c $< -o $@

$(FILE_UNIT_OBJ): $(FILE_UNIT_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(CONVERSION_UNIT_OBJ): $(CONVERSION_UNIT_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(MAIN_OBJ): $(MAIN_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(PARSER_OBJ) $(BUFFER_MGR_OBJ) $(CORE_OBJ) \
           $(FILE_UNIT_OBJ) $(CONVERSION_UNIT_OBJ) \
           $(MAIN_OBJ)
	$(CC) $(CFLAGS) $^ \
		$(VF_FILE_LIB) \
		$(VF_CONVERSION_LIB) \
		$(VF_FRAMEBUFFER_LIB) \
		$(VF_LOGGER_LIB) \
		$(VF_COMMON_LIB) \
		-lpthread \
		-o $@
	@echo "[service] Built $(TARGET)"

run: all
	cd $(PROJECT_ROOT) && ./service/build/vf_service

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "[service] Cleaning dependencies..."
	$(MAKE) -C $(VF_CONVERSION_LIB_DIR) clean
	$(MAKE) -C $(VF_FILE_LIB_DIR) clean
	$(MAKE) -C $(VF_FRAMEBUFFER_LIB_DIR) clean
	$(MAKE) -C $(VF_LOGGER_LIB_DIR) clean
	$(MAKE) -C $(VF_COMMON_LIB_DIR) clean
	rm -f $(BUILD_DIR)/*.o
	rm -f $(TARGET)
	@echo "[service] clean done."

