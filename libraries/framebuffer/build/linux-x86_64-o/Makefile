# Root directory (de la build/linux-x86_64-o pana la radacina proiectului)
PROJECT_ROOT := ../../../..

# Directories
SRC_DIR      := $(PROJECT_ROOT)/libraries/framebuffer/src
INC_DIR      := $(PROJECT_ROOT)/libraries/framebuffer/public
BUILD_DIR    := $(PROJECT_ROOT)/libraries/framebuffer/build/linux-x86_64-o
# TEST_DIR     := $(PROJECT_ROOT)/test_clients/framebuffer

# Common dependencies
COMMON_INC   := $(PROJECT_ROOT)/libraries/vf-common/public
COMMON_LIB   := $(PROJECT_ROOT)/libraries/vf-common/build/libvf-common.a

# Logger dependencies
LOGGER_INC   := $(PROJECT_ROOT)/libraries/vf-logger/public
LOGGER_LIB   := $(PROJECT_ROOT)/libraries/vf-logger/build/libvf-logger.a

# Service dependencies
SERVICE_INC  := $(PROJECT_ROOT)/service/public-inc

# Compiler
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -I$(INC_DIR) -I$(COMMON_INC) -I$(LOGGER_INC) -I$(SERVICE_INC)

# Framebuffer library files
FRAMEBUFFER_SRC := $(SRC_DIR)/vf-framebuffer.c
FRAMEBUFFER_OBJ := $(BUILD_DIR)/vf-framebuffer.o
FRAMEBUFFER_LIB := $(BUILD_DIR)/libvf-framebuffer.a

# Test client (optional)
# TEST_SRC := $(TEST_DIR)/main.c
# TEST_BIN := $(BUILD_DIR)/test_framebuffer

.PHONY: all clean run

# -------------------- Top rule --------------------
all: $(COMMON_LIB) $(LOGGER_LIB) $(FRAMEBUFFER_LIB)

# -------------------- Dep libs --------------------
$(COMMON_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-common/build

$(LOGGER_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-logger/build

# -------------------- Framebuffer lib --------------------
$(FRAMEBUFFER_OBJ): $(FRAMEBUFFER_SRC) $(INC_DIR)/vf-framebuffer.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FRAMEBUFFER_LIB): $(FRAMEBUFFER_OBJ)
	ar rcs $@ $<

# -------------------- Utility targets --------------------
clean:
	rm -f $(BUILD_DIR)/*.o $(FRAMEBUFFER_LIB) $(TEST_BIN)
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-common/build clean
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-logger/build clean

run: all
	@echo "Nu exista inca test_framebuffer (TEST_BIN e comentat)."
