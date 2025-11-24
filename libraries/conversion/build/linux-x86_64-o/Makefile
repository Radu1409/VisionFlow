# Root directory (de la build/linux-x86_64-o pana la radacina proiectului)
PROJECT_ROOT := ../../../..

# Directories
SRC_DIR      := $(PROJECT_ROOT)/libraries/conversion/src
INC_DIR      := $(PROJECT_ROOT)/libraries/conversion/public
BUILD_DIR    := $(PROJECT_ROOT)/libraries/conversion/build/linux-x86_64-o
TEST_DIR     := $(PROJECT_ROOT)/test_clients/conversion

# Common dependencies
COMMON_INC   := $(PROJECT_ROOT)/libraries/vf-common/public
COMMON_LIB   := $(PROJECT_ROOT)/libraries/vf-common/build/libvf-common.a

# Logger dependencies
LOGGER_INC   := $(PROJECT_ROOT)/libraries/vf-logger/public
LOGGER_LIB   := $(PROJECT_ROOT)/libraries/vf-logger/build/libvf-logger.a

# Framebuffer dependencies
FRAMEBUFFER_INC   := $(PROJECT_ROOT)/libraries/framebuffer/public
FRAMEBUFFER_LIB   := $(PROJECT_ROOT)/libraries/framebuffer/build/linux-x86_64-o/libvf-framebuffer.a

# Compiler
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -I$(INC_DIR) -I$(COMMON_INC) -I$(LOGGER_INC) -I$(FRAMEBUFFER_INC)

# Conversion library files
CONVERSION_SRC := $(SRC_DIR)/vf-conversion.c
CONVERSION_OBJ := $(BUILD_DIR)/vf-conversion.o
CONVERSION_LIB := $(BUILD_DIR)/libvf-conversion.a

# Test client
TEST_SRC := $(TEST_DIR)/main.c
TEST_BIN := $(BUILD_DIR)/test_conversion

.PHONY: all clean run

# -------------------- Top rule --------------------
all: $(COMMON_LIB) $(LOGGER_LIB) $(FRAMEBUFFER_LIB) $(CONVERSION_LIB) $(TEST_BIN)

# -------------------- Dep libs --------------------
$(COMMON_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-common/build

$(LOGGER_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-logger/build

$(FRAMEBUFFER_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/framebuffer/build/linux-x86_64-o/

# -------------------- Conversion lib --------------------
$(CONVERSION_OBJ): $(CONVERSION_SRC) $(INC_DIR)/vf-conversion.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CONVERSION_LIB): $(CONVERSION_OBJ)
	ar rcs $@ $<

# -------------------- Test binary --------------------
$(TEST_BIN): $(TEST_SRC) $(CONVERSION_LIB) $(COMMON_LIB) $(LOGGER_LIB) $(FRAMEBUFFER_LIB)
	$(CC) $(CFLAGS) -o $@ $^

# -------------------- Utility targets --------------------
clean:
	rm -f $(BUILD_DIR)/*.o $(CONVERSION_LIB) $(TEST_BIN)
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-common/build clean
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-logger/build clean
	$(MAKE) -C $(PROJECT_ROOT)/libraries/framebuffer/build/linux-x86_64-o/ clean

run: all
	./$(TEST_BIN)
