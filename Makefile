CXX      := g++
CXXFLAGS := -Wall -O3 -std=c++17
SHELL    := C:/msys64/usr/bin/sh.exe

RAYLIB_PATH := raylib

SRC_DIR   := src
BUILD_DIR := build

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC := $(call rwildcard,$(SRC_DIR)/,*.cc)
SRC := $(filter-out $(SRC_DIR)/Network/Tests/%,$(SRC))
OBJ := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

TARGET := game.exe

INCLUDES := -I$(SRC_DIR) -I$(RAYLIB_PATH)/include
LIBPATHS := -L$(RAYLIB_PATH)/lib
LDFLAGS  := -lraylib -lopengl32 -lenet -lws2_32 -lgdi32 -lwinmm -static-libgcc -static-libstdc++

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBPATHS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) nettest.exe lobbytest.exe

NET_DIR       := $(SRC_DIR)/Network
NET_FLAGS     := -Wall -std=c++17 -g -I$(SRC_DIR)
NET_LIBS      := -lenet -lws2_32 -lwinmm

TRANSPORT_SRC := $(NET_DIR)/Transport/EnetTransport.cc \
                 $(NET_DIR)/Transport/LagSimTransport.cc
PROTOCOL_SRC  := $(NET_DIR)/Protocol/ByteStream.cc \
                 $(NET_DIR)/Protocol/NetworkProtocol.cc

network_tests: nettest.exe lobbytest.exe

nettest.exe: $(NET_DIR)/Test/NetTest.cc \
             $(NET_DIR)/Protocol/ByteStream.cc \
             $(TRANSPORT_SRC)
	$(CXX) $(NET_FLAGS) -o $@ $^ $(NET_LIBS)

lobbytest.exe: $(NET_DIR)/Test/LobbyTest.cc \
               $(NET_DIR)/NetworkManager.cc \
               $(PROTOCOL_SRC) $(TRANSPORT_SRC)
	$(CXX) $(NET_FLAGS) -o $@ $^ $(NET_LIBS)

-include $(DEP)

.PHONY: all clean network_tests