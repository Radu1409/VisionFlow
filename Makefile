# Directories for camera and vf-logger
BUILD_CAMERA := libraries/camera/build
BUILD_VF_LOGGER := libraries/vf-logger/build
BUILD_VF_COMMON := libraries/vf-common/build
BUILD := build

# Main rule
all: common logger camera test_vf_service

# Call make on submodule
common:
	$(MAKE) -C $(BUILD_VF_COMMON)

logger:
	$(MAKE) -C $(BUILD_VF_LOGGER)

camera:
	$(MAKE) -C $(BUILD_CAMERA)

# Create test_vf_service main binary
test_vf_service: common logger camera $(BUILD)/test_vf_service

$(BUILD)/test_vf_service: $(BUILD_VF_COMMON)/libvf-common.a $(BUILD_VF_LOGGER)/libvf-logger.a $(BUILD_CAMERA)/libcamera.a build/main.o
	$(CC) -o $@ $^ -L$(BUILD_VF_COMMON) -lvf-common -L$(BUILD_VF_LOGGER) -lvf-logger -L$(BUILD_CAMERA) -lcamera

INCLUDES := -I libraries/vf-common/public -I libraries/vf-logger/public -I libraries/camera/public

# Rule to compile main.c
build/main.o: main.c
	$(CC) -Wall -Wextra $(INCLUDES) -c $< -o $@

# Clean
clean:
	$(MAKE) -C $(BUILD_VF_COMMON) clean
	$(MAKE) -C $(BUILD_VF_LOGGER) clean
	$(MAKE) -C $(BUILD_CAMERA) clean
	rm -f $(BUILD)/test_vf_service build/main.o

# Running application
run: test_vf_service
	./$(BUILD)/test_vf_service


# Directories for camera and vf-logger
# BUILD_CAMERA := libraries/camera/build
# BUILD_VF_LOGGER := libraries/vf-logger/build

# # Run Makefile from subdirectories
# all: logger camera

# # Call make on submodule
# logger:
# 	$(MAKE) -C $(BUILD_VF_LOGGER)

# camera:
# 	$(MAKE) -C $(BUILD_CAMERA)

# clean:
# 	$(MAKE) -C $(BUILD_VF_LOGGER) clean
# 	$(MAKE) -C $(BUILD_CAMERA) clean

# run: all
# 	$(MAKE) -C $(BUILD_VF_LOGGER) run
# 	$(MAKE) -C $(BUILD_CAMERA) run

