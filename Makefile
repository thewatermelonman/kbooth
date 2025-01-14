.PHONY: all clean run

SRC_DIR = src
LIB_DIR = lib/imgui
BUILD_DIR = build
TARGET = $(BUILD_DIR)/program

CC = g++
CFLAGS = -g -Wall -W -pedantic -I"C:\Program Files (x86)\SDL3\include" -Ilib/imgui
LDFLAGS = -L"C:\Program Files (x86)\SDL3\lib" -lSDL3

SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(LIB_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Rules
all: $(BUILD_DIR) $(TARGET)

run: all
	$(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
