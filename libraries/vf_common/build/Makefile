PROJECT_ROOT := ../../..

# Directories
SRC_DIR   := $(PROJECT_ROOT)/libraries/vf_common/src
INC_DIR   := $(PROJECT_ROOT)/libraries/vf_common/public
BUILD_DIR := $(PROJECT_ROOT)/libraries/vf_common/build

# Compiler
CC    := gcc
AR    := ar
CFLAGS := -Wall -Wextra -Werror -I$(INC_DIR)

# Files
COMMON_SRC := $(SRC_DIR)/vf-error.c
COMMON_OBJ := $(BUILD_DIR)/vf-error.o
COMMON_LIB := $(BUILD_DIR)/libvf-common.a

# ==============================================================================
# Rules
# ==============================================================================

.PHONY: all clean $(COMMON_OBJ) $(COMMON_LIB)

all: $(BUILD_DIR) $(COMMON_LIB)

$(COMMON_OBJ): $(COMMON_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(COMMON_LIB): $(COMMON_OBJ)
	$(AR) rcs $@ $^
	@echo "[vf_common] libvf-common.a built successfully."

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -f $(BUILD_DIR)/*.o $(COMMON_LIB)
	@echo "[vf_common] clean done."

