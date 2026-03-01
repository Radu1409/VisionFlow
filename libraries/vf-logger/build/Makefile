# # Root directory
# PROJECT_ROOT := ../../..

# # Directories
# SRC_DIR := $(PROJECT_ROOT)/libraries/vf-logger/src
# INC_DIR := $(PROJECT_ROOT)/libraries/vf-logger/public
# BUILD_DIR := $(PROJECT_ROOT)/libraries/vf-logger/build
# TEST_DIR := $(PROJECT_ROOT)/test_clients/vf-logger

# # Dependencies
# VF_COMMON_INC_DIR := $(PROJECT_ROOT)/libraries/vf-common/public

# # Compilator
# CC := gcc
# CFLAGS := -Wall -Wextra -I$(INC_DIR) -I$(VF_COMMON_INC_DIR)

# # Files
# LOGGER_SRC := $(SRC_DIR)/vf-logger.c
# LOGGER_OBJ := $(BUILD_DIR)/vf-logger.o
# LOGGER_LIB := $(BUILD_DIR)/libvf-logger.a  # Static library

# TEST_SRC := $(TEST_DIR)/main.c
# TEST_BIN := $(BUILD_DIR)/test_logger

# # Rules
# all: $(LOGGER_LIB) $(TEST_BIN)

# $(LOGGER_OBJ): $(LOGGER_SRC)
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(LOGGER_LIB): $(LOGGER_OBJ)
# 	ar rcs $@ $<

# $(TEST_BIN): $(TEST_SRC) $(LOGGER_LIB)
# 	$(CC) $(CFLAGS) -o $@ $^

# clean:
# 	rm -f $(BUILD_DIR)/*.o $(LOGGER_LIB) $(TEST_BIN)

# run: all
# 	./$(TEST_BIN)



# Root directory
PROJECT_ROOT := ../../..

# Directories
SRC_DIR := $(PROJECT_ROOT)/libraries/vf-logger/src
INC_DIR := $(PROJECT_ROOT)/libraries/vf-logger/public
BUILD_DIR := $(PROJECT_ROOT)/libraries/vf-logger/build
TEST_DIR := $(PROJECT_ROOT)/test_clients/vf-logger

# Dependencies
VF_COMMON_INC_DIR := $(PROJECT_ROOT)/libraries/vf-common/public
VF_COMMON_LIB_DIR := $(PROJECT_ROOT)/libraries/vf-common/build

# Compilator
CC := gcc
CFLAGS := -Wall -Wextra -I$(INC_DIR) -I$(VF_COMMON_INC_DIR)

# Files for Logger
LOGGER_SRC := $(SRC_DIR)/vf-logger.c
LOGGER_OBJ := $(BUILD_DIR)/vf-logger.o
LOGGER_LIB := $(BUILD_DIR)/libvf-logger.a  # Static library

# Files for Common (Error handling)
COMMON_LIB := $(VF_COMMON_LIB_DIR)/libvf-common.a

# Files for Test
TEST_SRC := $(TEST_DIR)/main.c
TEST_BIN := $(BUILD_DIR)/test_logger

# Rules
all: $(LOGGER_LIB) $(TEST_BIN)

$(LOGGER_OBJ): $(LOGGER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# New rule: how to compile vf-error.c in vf-error.o
$(ERROR_OBJ): $(ERROR_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(LOGGER_LIB): $(LOGGER_OBJ)
	ar rcs $@ $<

$(TEST_BIN): $(TEST_SRC) $(ERROR_OBJ) $(LOGGER_LIB) $(COMMON_LIB)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(BUILD_DIR)/*.o $(LOGGER_LIB) $(TEST_BIN)

run: all
	./$(TEST_BIN)