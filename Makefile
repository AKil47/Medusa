# Medusa Plugin Makefile
# Handles CMake build process for Windows environment

BUILD_DIR = build
BIN_DIR = bin

.PHONY: all build clean configure

all: build

configure:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	cd $(BUILD_DIR) && cmake ../native -G "NMake Makefiles"

build: configure
	cd $(BUILD_DIR) && nmake

clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)

rebuild: clean build