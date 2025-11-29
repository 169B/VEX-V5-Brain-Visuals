# Makefile for VEX V5 Host Mode Simulator
# Compiles the C++ code that simulates the VEX V5 Brain

# Compiler settings
CXX := g++
CXXFLAGS := -std=c++17 -DHOST_MODE -Wall -Wextra -g -Iinclude -Iinclude/liblvgl

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    LDFLAGS := -lpthread -lws2_32
    TARGET := bin/host_brain.exe
    RM := del /Q
    MKDIR := mkdir
else
    LDFLAGS := -lpthread
    TARGET := bin/host_brain
    RM := rm -rf
    MKDIR := mkdir -p
endif

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# Source files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Default target
.PHONY: all
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	@$(MKDIR) $(BIN_DIR) 2>/dev/null || true
	$(CXX) $^ -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(MKDIR) $(dir $@) 2>/dev/null || true
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependencies
-include $(DEPS)

# Clean build files
.PHONY: clean
clean:
	$(RM) $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

# Run the host brain
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Install Node.js dependencies for UI
.PHONY: ui-install
ui-install:
	cd ui && npm install

# Start the UI server
.PHONY: ui-start
ui-start:
	cd ui && npm start

# Full build and run with UI
.PHONY: start
start: all ui-install
	@echo "Starting UI server in background..."
	cd ui && npm start &
	@sleep 2
	@echo "Starting host brain..."
	./$(TARGET)

# Help
.PHONY: help
help:
	@echo "VEX V5 Host Mode Simulator - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build the host brain executable (default)"
	@echo "  clean      - Remove build files"
	@echo "  run        - Build and run the host brain"
	@echo "  ui-install - Install UI dependencies (npm)"
	@echo "  ui-start   - Start the UI server"
	@echo "  start      - Full build and run with UI"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  1. Run 'make' to build the executable"
	@echo "  2. Run 'make ui-install' to install UI dependencies"
	@echo "  3. In one terminal: 'make ui-start'"
	@echo "  4. In another terminal: 'make run'"
	@echo "  5. Open http://localhost:3000 in your browser"
