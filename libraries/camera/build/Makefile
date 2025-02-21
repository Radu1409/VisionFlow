# Root directory
PROJECT_ROOT := ../../..

# Directories
SRC_DIR := $(PROJECT_ROOT)/libraries/camera/src
INC_DIR := $(PROJECT_ROOT)/libraries/camera/public
BUILD_DIR := $(PROJECT_ROOT)/libraries/camera/build
TEST_DIR := $(PROJECT_ROOT)/test_clients/camera

# Common dependencies
COMMON_INC := $(PROJECT_ROOT)/libraries/vf-common/public
COMMON_LIB := $(PROJECT_ROOT)/libraries/vf-common/build/libvf-common.a

# Logger dependencies
LOGGER_INC := $(PROJECT_ROOT)/libraries/vf-logger/public
LOGGER_LIB := $(PROJECT_ROOT)/libraries/vf-logger/build/libvf-logger.a

# Compilator
CC := gcc
CFLAGS := -Wall -Wextra -I$(INC_DIR) -I$(COMMON_INC) -I$(LOGGER_INC)

# Files
CAMERA_SRC := $(SRC_DIR)/camera.c
CAMERA_OBJ := $(BUILD_DIR)/camera.o
CAMERA_LIB := $(BUILD_DIR)/libcamera.a  # Static library

TEST_SRC := $(TEST_DIR)/main.c
TEST_BIN := $(BUILD_DIR)/test_camera

# Rules
all: $(COMMON_LIB) $(LOGGER_LIB) $(CAMERA_LIB) $(TEST_BIN)

$(COMMON_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-common/build

$(LOGGER_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-logger/build

$(CAMERA_OBJ): $(CAMERA_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(CAMERA_LIB): $(CAMERA_OBJ)
	ar rcs $@ $<

$(TEST_BIN): $(TEST_SRC) $(CAMERA_LIB) $(COMMON_LIB) $(LOGGER_LIB)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(BUILD_DIR)/*.o $(CAMERA_LIB) $(TEST_BIN)
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-common/build clean
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-logger/build clean

run: all
	./$(TEST_BIN)

