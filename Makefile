# --- CUSTOMIZE YOUR GAME NAMES HERE ---
NAME ?= mini_game
TITLE ?= Mini Game

# --- DEFAULT BUILD SETTING (Now defaults to fast Debug) ---
BUILD ?= Debug

# --- Asset Packing Configurations ---
ASSETS_DIR := assets
ASSET_SRCS := $(shell find $(ASSETS_DIR) -type f)
ASSETS_SCRIPT_SRC := toolchain/src/pack_assets.cr

IMAGE_HEADER := src/assets/ImageData.h
MUSIC_HEADER := src/assets/MusicData.h
ASSET_HEADERS := $(IMAGE_HEADER) $(MUSIC_HEADER)

.PHONY: all assets build run clean build-release run-release clean-release release

# DEFAULT WORKFLOW
all: assets build run

# Grouped rule: tells Make that one invocation of the command generates ALL listed headers
$(ASSET_HEADERS) &: $(ASSET_SRCS) $(ASSETS_SCRIPT_SRC)
	@echo "--- Asset updates detected! Re-running packer pipeline ---"
	@cd toolchain && crystal run src/pack_assets.cr

# Explicit target for manual 'make assets' invocation
assets: $(ASSET_HEADERS)

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
