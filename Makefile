CXX      := g++
CXXFLAGS := -Wall -O3 -std=c++17
SHELL    := C:/msys64/usr/bin/sh.exe

RAYLIB_PATH := raylib

SRC_DIR   := src
BUILD_DIR := build

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC := $(call rwildcard,$(SRC_DIR)/,*.cc)
OBJ := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRC))

TARGET := game.exe

INCLUDES := -I$(SRC_DIR) -I$(RAYLIB_PATH)/include
LIBPATHS := -L$(RAYLIB_PATH)/lib
LDFLAGS  := -lraylib -lopengl32 -lgdi32 -lwinmm -static-libgcc -static-libstdc++

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBPATHS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean