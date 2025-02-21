# Root directory
PROJECT_ROOT := ../../..

# Directories
SRC_DIR := $(PROJECT_ROOT)/libraries/vf-common/src
INC_DIR := $(PROJECT_ROOT)/libraries/vf-common/public
BUILD_DIR := $(PROJECT_ROOT)/libraries/vf-common/build
TEST_DIR := $(PROJECT_ROOT)/test_clients/vf-common

# Compilator
CC := gcc
CFLAGS := -Wall -Wextra -I$(INC_DIR)

# Files
COMMON_SRC := $(SRC_DIR)/vf-error.c
COMMON_OBJ := $(BUILD_DIR)/vf-error.o
COMMON_LIB := $(BUILD_DIR)/libvf-common.a  # Static library

TEST_SRC := $(TEST_DIR)/main.c
TEST_BIN := $(BUILD_DIR)/test_vf_common

# Rules
all: $(COMMON_LIB) $(TEST_BIN)

$(COMMON_OBJ): $(COMMON_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(COMMON_LIB): $(COMMON_OBJ)
	ar rcs $@ $<

$(TEST_BIN): $(TEST_SRC) $(COMMON_LIB)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(BUILD_DIR)/*.o $(COMMON_LIB) $(TEST_BIN)

run: all
	./$(TEST_BIN)

