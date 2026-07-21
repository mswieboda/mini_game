# --- CUSTOMIZE YOUR GAME NAMES HERE ---
NAME ?= mini_game
TITLE ?= Mini Game

# --- DEFAULT BUILD SETTING (Now defaults to fast Debug) ---
BUILD ?= Debug

# --- Asset Packing Configurations ---
ASSETS_DIR := assets
ASSET_SRCS := $(shell find $(ASSETS_DIR) -type f 2>/dev/null)
ASSETS_SCRIPT_SRC := toolchain/src/pack_assets.cr
ASSETS_SCRIPTS := $(ASSETS_SCRIPT_SRC) $(shell find toolchain/src/export -type f 2>/dev/null)
ASSETS_STAMP := build/.assets.stamp

.PHONY: all assets build run clean build-release run-release clean-release release

# DEFAULT WORKFLOW
all: build run

# The script runs ONLY ONCE when any asset file or the script itself changes
$(ASSETS_STAMP): $(ASSET_SRCS) $(ASSETS_SCRIPT_SRC) $(ASSETS_SCRIPTS)
	@mkdir -p build
	@echo "--- Asset updates detected! Re-running packer pipeline ---"
	@crystal run $(ASSETS_SCRIPT_SRC)
	@touch $(ASSETS_STAMP)

# Phony 'assets' target points to the stamp file
assets: $(ASSETS_STAMP)

build:
	@echo "--- Compiling [$(NAME) | Mode: $(BUILD)] ---"
	@# Only run CMake generation if the build folder doesn't exist yet
	@if [ ! -d "build/$(BUILD)" ]; then \
		cmake -B build/$(BUILD) -DCMAKE_BUILD_TYPE=$(BUILD) -DGAME_BIN=$(NAME) -DGAME_TITLE="$(TITLE)"; \
	fi
	@# Use parallel core compilation (-j) to unleash all your CPU cores!
	@cmake --build build/$(BUILD) -j

run:
	@echo "--- Running [$(NAME) | Mode: $(BUILD)] ---"
	@./build/$(BUILD)/$(NAME)

clean:
	@echo "--- Cleaning [$(BUILD)] Workspace ---"
	@rm -rf build/$(BUILD)
	@rm -rf build/.assets.stamp
	@rm -rf toolchain/build/*
	@rm -rf src/assets/*

# EXPLICIT RELEASE SHORTCUTS (Overrides the BUILD variable to Release)
build-release:
	@$(MAKE) build BUILD=Release

run-release:
	@$(MAKE) run BUILD=Release

clean-release:
	@$(MAKE) clean BUILD=Release

release:
	@$(MAKE) clean BUILD=Release assets build-release run-release
