SRC_DIR := ./src
INC_DIR := ./inc
OBJ_DIR := ./obj
BUILD_DIR := ./build
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

LIB_FLAGS = -lSDL2
ifeq ($(shell uname),Darwin) # macOS
	detected_OS := macOS
	INC_DIR += -I /opt/homebrew/include
    LIB_FLAGS += -L /opt/homebrew/lib
endif

CXX = g++
CXX_FLAGS = -Wall -std=c++20 -I $(INC_DIR)

all: release

release: CXX_FLAGS += -O3
release: BUILD_NAME=emulator
release: executable

debug: CXX_FLAGS += -DDEBUG -g3
debug: BUILD_NAME=emulator_debug
debug: executable

executable: $(OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(LIB_FLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) $(OBJ_FILES)

# Generic rules for compiling a source file to an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_FILES) $(BUILD_DIR)/*