# --- CUSTOMIZE YOUR GAME NAMES HERE ---
NAME ?= mini_game
TITLE ?= Mini Game

# --- DEFAULT BUILD SETTING (Now defaults to fast Debug) ---
BUILD ?= Debug

# Automatically scans the assets folder for all .aseprite sheets
ASSET_SRCS := $(wildcard assets/*.aseprite)
ASSETS_SCRIPT_SRC := toolchain/src/pack_assets.cr
ASSETS_HEADER := src/assets.h

.PHONY: all build run clean build-release run-release clean-release

# DEFAULT WORKFLOW
all: $(ASSETS_HEADER) build run

# The Makefile checks the timestamps of ALL matching files inside $(ASSET_SRCS)
$(ASSETS_HEADER): $(ASSET_SRCS) $(ASSETS_SCRIPT_SRC)
	@echo "--- Asset updates detected! Re-running packer pipeline ---"
	cd toolchain && crystal run src/pack_assets.cr

assets:
	@echo "--- Packing Assets via Aseprite ---"
	cd toolchain && crystal run src/pack_assets.cr

build:
	@echo "--- Compiling [$(NAME) | Mode: $(BUILD)] ---"
	# Only run CMake generation if the build folder doesn't exist yet
	@if [ ! -d "build/$(BUILD)" ]; then \
		cmake -B build/$(BUILD) -DCMAKE_BUILD_TYPE=$(BUILD) -DGAME_BIN=$(NAME) -DGAME_TITLE="$(TITLE)"; \
	fi
	# Use parallel core compilation (-j) to unleash all your CPU cores!
	cmake --build build/$(BUILD) -j

run:
	@echo "--- Running [$(NAME) | Mode: $(BUILD)] ---"
	./build/$(BUILD)/$(NAME)

clean:
	@echo "--- Cleaning [$(BUILD)] Workspace ---"
	rm -rf build/$(BUILD)
	rm -rf toolchain/bin
	rm -f src/assets.h


# EXPLICIT RELEASE SHORTCUTS (Overrides the BUILD variable to Release)
build-release:
	@$(MAKE) build BUILD=Release

run-release:
	@$(MAKE) run BUILD=Release

clean-release:
	@$(MAKE) clean BUILD=Release
