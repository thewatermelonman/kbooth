.PHONY: all clean run

SRC_DIR = src
LIB_DIR = lib/imgui
BUILD_DIR = build
TARGET = $(BUILD_DIR)/program

CC = g++
CFLAGS = -g -Wall -W -pedantic -I"C:\Program Files (x86)\SDL3\include" -I$(LIB_DIR)
LDFLAGS = -L"C:\Program Files (x86)\SDL3\lib" -lSDL3

SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(LIB_DIR)/*.cpp) 
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run: all
	$(TARGET)

# Clean up
clean:
	rm -rf $(BUILD_DIR)

