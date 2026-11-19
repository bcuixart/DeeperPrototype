CXX      := g++
CXXFLAGS := -Wall -O3 -std=c++17

# ---------------------------------------------------------------------------
# Platform detection. Windows sets OS=Windows_NT in the environment; anything
# else is treated as Linux. Override manually with: make PLATFORM=linux|windows
# ---------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
    PLATFORM ?= windows
else
    PLATFORM ?= linux
endif

RAYLIB_PATH := raylib

ifeq ($(PLATFORM),windows)
    SHELL    := C:/msys64/usr/bin/sh.exe
    EXE      := .exe
    LIBPATHS := -L$(RAYLIB_PATH)/lib
    LDFLAGS  := -lraylib -lopengl32 -lenet -lws2_32 -lgdi32 -lwinmm -static-libgcc -static-libstdc++
    NET_LIBS := -lenet -lws2_32 -lwinmm
else
    EXE      :=
    LIBPATHS := -L$(RAYLIB_PATH)/lib_linux
    LDFLAGS  := -lraylib -lenet -lGL -lm -lpthread -ldl -lrt -lX11
    NET_LIBS := -lenet
endif

SRC_DIR   := src
# One build folder per platform so Windows and Linux objects never mix
BUILD_DIR := build/$(PLATFORM)

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC := $(call rwildcard,$(SRC_DIR)/,*.cc)
SRC := $(filter-out $(SRC_DIR)/Network/Tests/%,$(SRC))
SRC := $(filter-out $(SRC_DIR)/Level/PhysicsTest/%,$(SRC))
OBJ := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

TARGET := game$(EXE)

INCLUDES := -I$(SRC_DIR) -I$(RAYLIB_PATH)/include

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBPATHS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

clean:
	rm -rf build $(TARGET) nettest$(EXE) lobbytest$(EXE) physicstest$(EXE) fuzztest$(EXE)
	rm -f game.exe game nettest.exe lobbytest.exe physicstest.exe fuzztest.exe nettest lobbytest physicstest fuzztest

# ---------------------------------------------------------------------------
# Physics test
# ---------------------------------------------------------------------------
TEST_DIR   := $(SRC_DIR)/Level/PhysicsTest
TEST_OBJ   := $(BUILD_DIR)/tests/PhysicsTest.o

# On Windows "make physicstest" is an alias of physicstest.exe. On Linux the
# target already has that exact name, so no alias is needed (it would be circular).
ifneq ($(EXE),)
physicstest: physicstest$(EXE)
.PHONY: physicstest
endif

GAME_OBJ_NO_MAIN := $(filter-out $(BUILD_DIR)/main.o,$(OBJ))

physicstest$(EXE): $(GAME_OBJ_NO_MAIN) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBPATHS) $(LDFLAGS)

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

-include $(TEST_OBJ:.o=.d)

# ---------------------------------------------------------------------------
# Network tests
# ---------------------------------------------------------------------------
NET_DIR       := $(SRC_DIR)/Network
NET_FLAGS     := -Wall -std=c++17 -g -I$(SRC_DIR)

TRANSPORT_SRC := $(NET_DIR)/Transport/EnetTransport.cc \
                 $(NET_DIR)/Transport/LagSimTransport.cc
PROTOCOL_SRC  := $(NET_DIR)/Protocol/ByteStream.cc \
                 $(NET_DIR)/Protocol/NetworkProtocol.cc

network_tests: nettest$(EXE) lobbytest$(EXE) fuzztest$(EXE)

nettest$(EXE): $(NET_DIR)/Tests/NetTest.cc \
               $(NET_DIR)/Protocol/ByteStream.cc \
               $(TRANSPORT_SRC)
	$(CXX) $(NET_FLAGS) -o $@ $^ $(NET_LIBS)

lobbytest$(EXE): $(NET_DIR)/Tests/LobbyTest.cc \
                 $(NET_DIR)/NetworkManager.cc \
                 $(PROTOCOL_SRC) $(TRANSPORT_SRC)
	$(CXX) $(NET_FLAGS) -o $@ $^ $(NET_LIBS)

fuzztest$(EXE): $(NET_DIR)/Tests/FuzzTest.cc \
                $(NET_DIR)/Protocol/ByteStream.cc \
                $(SRC_DIR)/Level/MatchManager.cc \
                $(SRC_DIR)/Level/RoundManager.cc \
                $(PROTOCOL_SRC) $(TRANSPORT_SRC)
	$(CXX) $(NET_FLAGS) -o $@ $^ $(NET_LIBS)

-include $(DEP)

.PHONY: all clean network_tests