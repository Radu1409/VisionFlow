# Root directory (de la build/linux-x86_64-o pana la radacina proiectului)
PROJECT_ROOT := ../../..

# Directoare service
SERVICE_SRC_DIR   := $(PROJECT_ROOT)/service/src
SERVICE_BUILD_DIR := $(PROJECT_ROOT)/service/build/linux-x86_64-o

# Directoare cu submodulele din service/src
SERVICE_CORE_DIR      := $(SERVICE_SRC_DIR)/core
SERVICE_PARSER_DIR    := $(SERVICE_SRC_DIR)/parser
SERVICE_UNITS_DIR     := $(SERVICE_SRC_DIR)/units
SERVICE_CAMERA_UNIT   := $(SERVICE_UNITS_DIR)/camera
SERVICE_CONV_UNIT     := $(SERVICE_UNITS_DIR)/conversion
SERVICE_DISPLAY_UNIT  := $(SERVICE_UNITS_DIR)/display
SERVICE_ENCODER_UNIT  := $(SERVICE_UNITS_DIR)/encoder

# Librarii comune
COMMON_INC   := $(PROJECT_ROOT)/libraries/vf-common/public
COMMON_LIB   := $(PROJECT_ROOT)/libraries/vf-common/build/libvf-common.a

LOGGER_INC   := $(PROJECT_ROOT)/libraries/vf-logger/public
LOGGER_LIB   := $(PROJECT_ROOT)/libraries/vf-logger/build/libvf-logger.a

FRAMEBUFFER_INC := $(PROJECT_ROOT)/libraries/framebuffer/public
FRAMEBUFFER_LIB := $(PROJECT_ROOT)/libraries/framebuffer/build/linux-x86_64-o/libvf-framebuffer.a

CONVERSION_INC := $(PROJECT_ROOT)/libraries/conversion/public
CONVERSION_LIB := $(PROJECT_ROOT)/libraries/conversion/build/linux-x86_64-o/libvf-conversion.a

# (optional, pentru viitor)
# VF_FILE_INC := $(PROJECT_ROOT)/libraries/vf-file/public-inc
# VF_FILE_LIB := $(PROJECT_ROOT)/libraries/vf-file/build/linux-x86_64-o/libvf-file.a

CAMERA_INC := $(PROJECT_ROOT)/libraries/camera/public
# CAMERA_LIB := $(PROJECT_ROOT)/libraries/camera/build/libcamera.a   # daca vei avea lib

# Compiler
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 \
          -I$(SERVICE_SRC_DIR) \
          -I$(SERVICE_CORE_DIR) \
          -I$(SERVICE_PARSER_DIR) \
          -I$(SERVICE_CAMERA_UNIT) \
          -I$(SERVICE_CONV_UNIT) \
          -I$(SERVICE_DISPLAY_UNIT) \
          -I$(SERVICE_ENCODER_UNIT) \
          -I$(COMMON_INC) \
          -I$(LOGGER_INC) \
          -I$(FRAMEBUFFER_INC) \
          -I$(CONVERSION_INC) \
          -I$(CAMERA_INC)

# Executabil
SERVICE_BIN := $(SERVICE_BUILD_DIR)/vf-service

# Surse service
SERVICE_SRCS := \
    $(SERVICE_CORE_DIR)/vf-core.c \
    $(SERVICE_CORE_DIR)/vf-pipeline-mgr.c \
    $(SERVICE_CORE_DIR)/vf-processing-unit.c \
    $(SERVICE_PARSER_DIR)/vf-parser.c \
    $(SERVICE_CAMERA_UNIT)/camera-unit.c \
    $(SERVICE_CONV_UNIT)/conversion-unit.c \
    $(SERVICE_DISPLAY_UNIT)/display-unit.c \
    $(SERVICE_ENCODER_UNIT)/encoder-unit.c

# Obiecte (le tinem toate in build/linux-x86_64-o)
SERVICE_OBJS := \
    $(SERVICE_BUILD_DIR)/vf-core.o \
    $(SERVICE_BUILD_DIR)/vf-pipeline-mgr.o \
    $(SERVICE_BUILD_DIR)/vf-processing-unit.o \
    $(SERVICE_BUILD_DIR)/vf-parser.o \
    $(SERVICE_BUILD_DIR)/camera-unit.o \
    $(SERVICE_BUILD_DIR)/conversion-unit.o \
    $(SERVICE_BUILD_DIR)/display-unit.o \
    $(SERVICE_BUILD_DIR)/encoder-unit.o

.PHONY: all clean run

# -------------------- Top rule --------------------
all: $(COMMON_LIB) $(LOGGER_LIB) $(FRAMEBUFFER_LIB) $(CONVERSION_LIB) $(SERVICE_BIN)

# -------------------- Dep libs --------------------
$(COMMON_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-common/build

$(LOGGER_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-logger/build

$(FRAMEBUFFER_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/framebuffer/build/linux-x86_64-o

$(CONVERSION_LIB):
	$(MAKE) -C $(PROJECT_ROOT)/libraries/conversion/build/linux-x86_64-o

# Daca vei avea si lib-uri vf-file/camera, adaugi aici reguli similare

# -------------------- Build obiecte service --------------------
$(SERVICE_BUILD_DIR)/vf-core.o: $(SERVICE_CORE_DIR)/vf-core.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVICE_BUILD_DIR)/vf-pipeline-mgr.o: $(SERVICE_CORE_DIR)/vf-pipeline-mgr.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVICE_BUILD_DIR)/vf-processing-unit.o: $(SERVICE_CORE_DIR)/vf-processing-unit.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVICE_BUILD_DIR)/vf-parser.o: $(SERVICE_PARSER_DIR)/vf-parser.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVICE_BUILD_DIR)/camera-unit.o: $(SERVICE_CAMERA_UNIT)/camera-unit.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVICE_BUILD_DIR)/conversion-unit.o: $(SERVICE_CONV_UNIT)/conversion-unit.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVICE_BUILD_DIR)/display-unit.o: $(SERVICE_DISPLAY_UNIT)/display-unit.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVICE_BUILD_DIR)/encoder-unit.o: $(SERVICE_ENCODER_UNIT)/encoder-unit.c
	$(CC) $(CFLAGS) -c $< -o $@

# -------------------- Link executabil --------------------
$(SERVICE_BIN): $(SERVICE_OBJS) $(COMMON_LIB) $(LOGGER_LIB) $(FRAMEBUFFER_LIB) $(CONVERSION_LIB)
	$(CC) $(CFLAGS) -o $@ $^

# -------------------- Utility targets --------------------
clean:
	rm -f $(SERVICE_BUILD_DIR)/*.o $(SERVICE_BIN)
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-common/build clean
	$(MAKE) -C $(PROJECT_ROOT)/libraries/vf-logger/build clean
	$(MAKE) -C $(PROJECT_ROOT)/libraries/framebuffer/build/linux-x86_64-o clean
	$(MAKE) -C $(PROJECT_ROOT)/libraries/conversion/build/linux-x86_64-o clean

run: all
	$(SERVICE_BIN)
