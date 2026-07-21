# --- CUSTOMIZE YOUR GAME NAMES HERE ---
NAME ?= mini_game
TITLE ?= Mini Game

# --- DEFAULT BUILD SETTING (Debug or Release) ---
BUILD ?= Debug
BUILD_DIR = build/$(BUILD)

# --- Asset Packing Configurations ---
ASSETS_DIR := assets
ASSET_SRCS := $(shell find $(ASSETS_DIR) -type f 2>/dev/null)
ASSETS_SCRIPT_SRC := toolchain/src/pack_assets.cr
ASSETS_SCRIPTS := $(ASSETS_SCRIPT_SRC) $(shell find toolchain/src/export -type f 2>/dev/null)
ASSETS_STAMP := build/.assets.stamp

.PHONY: all assets build run clean clean-assets build-release run-release clean-release release config reconfig reset

# DEFAULT WORKFLOW
all: build run

# The script runs ONLY when asset files or packer scripts change
$(ASSETS_STAMP): $(ASSET_SRCS) $(ASSETS_SCRIPTS)
	@mkdir -p build
	@if [ -n "$(ASSET_SRCS)" ]; then \
		echo "--- Asset updates detected! Re-running packer pipeline ---"; \
		crystal run $(ASSETS_SCRIPT_SRC); \
	fi
	@touch $(ASSETS_STAMP)

assets: $(ASSETS_STAMP)

build: assets
	@echo "--- Compiling [$(NAME) | Mode: $(BUILD)] ---"
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD) -DGAME_BIN=$(NAME) -DGAME_TITLE="$(TITLE)"; \
	fi
	@cmake --build $(BUILD_DIR) -j

run:
	@echo "--- Running [$(NAME) | Mode: $(BUILD)] ---"
	@./$(BUILD_DIR)/$(NAME)

clean:
	@echo "--- Cleaning [$(BUILD)] ---"
	@rm -rf $(BUILD_DIR)

clean-assets:
	@echo "--- Cleaning Generated Asset Stamp ---"
	@rm -rf build/.assets.stamp

# --- RELEASE SHORTCUTS ---
build-release:
	@$(MAKE) build BUILD=Release

run-release:
	@$(MAKE) run BUILD=Release

clean-release:
	@$(MAKE) clean BUILD=Release

release:
	@$(MAKE) BUILD=Release reset build-release run-release

# --- CMAKE CONFIG SHORTCUTS ---
config:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD) -DGAME_BIN=$(NAME) -DGAME_TITLE="$(TITLE)"

reconfig:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD) -DGAME_BIN=$(NAME) -DGAME_TITLE="$(TITLE)" --fresh

reset: clean reconfig
