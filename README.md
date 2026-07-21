# mini_game

Boilerplate project for a C++ `minifb` game, via macOS or Linux.

## 🧰 Toolchain & Dependencies

To build the executable and run the asset pipeline from scratch, ensure you have the required host dependencies installed.

### System Requirements Summary

* **C++ Compiler:** C++20 compatible (`clang++` or `g++`)
* **Build System:** `cmake` (>= 3.16) and `make`
* **Asset Pipeline:** `crystal` (v1.20.2 recommended)
* **Executable Packer:** `upx` (Optional for release builds, compresses binary size)
* **Pixel Art Editor:** `aseprite` (Optional, auto-exports sprites to C++ headers)

---

### macOS Setup

Install the required tools using [Homebrew](https://brew.sh/):

```bash
# Core build tools and compression utility
brew install cmake make upx crystal

# Optional: Version manager alternative if using .tool-versions (asdf / mise)
brew install asdf
```

---

### Linux (Ubuntu/Debian) Setup

1. **System Libraries & Build Tools:**

```bash
sudo apt update
sudo apt install -y build-essential cmake make libx11-dev libgl1-mesa-dev upx
```

2. **Crystal Language:**

Install Crystal via official APT repositories or snap:

```bash
# Via Snap:
sudo snap install crystal --classic

# Or follow official repository installation instructions at [https://crystal-lang.org/install/](https://crystal-lang.org/install/)
```

---

### Managing Crystal & Tools via `.tool-versions` (`asdf` / `mise`)

If you use a polyglot version manager like [`asdf`](https://asdf-vm.com/) or [`mise`](https://mise.jdx.dev/), a `toolchain/.tool-versions` file is provided to pin exact versions:

```text
crystal 1.20.2
```

To install the configured versions:

* **`asdf` Users:**
  ```bash
  asdf plugin add crystal
  asdf install
  ```

* **`mise` Users:**
  ```bash
  mise install
  ```

---

## 🚀 Installation & First Run

1. **Clone with Submodules:**

   ```bash
   git clone --recursive [https://github.com/mswieboda/mini_game.git](https://github.com/mswieboda/mini_game.git)
   cd mini_game
   ```

   *(If already cloned without submodules, run `git submodule update --init --recursive`)*

2. **Build and Run:**

   ```bash
   make
   ```

---

## ⚙️ Customizing Executable Name & Window Title

You can customize your executable output name and window title directly in the `Makefile` or via command-line arguments.

### Method A: Permanent Configuration (Edit Makefile)
At the top of the `Makefile`, edit the default values:
```makefile
NAME  ?= my_awesome_game
TITLE ?= My Awesome Game
```

### Method B: On-The-Fly Overrides
Pass variables directly when running `make`:
```bash
make NAME=space_shooter TITLE="Space Shooter v1.0"
```
Or for release builds:
```bash
make release NAME=space_shooter TITLE="Space Shooter v1.0"
```

## 🛠️ Asset Pipeline Documentation & Troubleshooting

The asset system uses a **Crystal** asset packer (`toolchain/src/pack_assets.cr`) that reads raw source files from the `assets/` folder and generates standard C++ headers directly into `src/assets/`:

| Source Folder | Input Formats | Generated Output Header |
| :--- | :--- | :--- |
| `assets/images/` | `.aseprite`, `.ase`, `.png` | `src/assets/images.h` |
| `assets/fonts/` | `.png`, `.bmp` | `src/assets/font_data.h` |
| `assets/audio/` | `.mod`, `.xm`, `.wav` | `src/assets/audio_data.h` |

The asset packer runs automatically during `make build` whenever changes to raw assets or toolchain scripts are detected.

---

### Option A: Using the Automated Aseprite Setup

The image packing step expects `aseprite` on your system path. On macOS, Aseprite is packaged in an application bundle directory (`/Applications/Aseprite.app`), so standard symlinks cause resource initialization failures. Follow this setup:

1. Remove any old symbolic references:
   ```bash
   rm -f ~/.local/bin/aseprite
   ```

2. Create a path-relative wrapper file at `~/.local/bin/aseprite` (or another path directory tracked in your `$PATH`):

   File `~/.local/bin/aseprite`:
   ```bash
   #!/bin/bash
   exec /Applications/Aseprite.app/Contents/MacOS/aseprite "$@"
   ```

3. Mark the script executable:
   ```bash
   chmod +x ~/.local/bin/aseprite
   ```

4. Verify terminal execution:
   ```bash
   aseprite --version
   ```

---

### Option B: Hand-Crafted or Alternative Asset Pipelines

If you don't use Aseprite, or prefer to write/generate your C++ asset arrays manually (e.g. using Libresprite, Piskel, GIMP, or custom scripts), you can bypass any part of the automated asset pipeline without breaking your build.

#### 1. Manual Image Data Layout (`src/assets/Images.h`)

The software renderer expects a palette lookup table and RLE pair runs:

```cpp
#pragma once
#include <cstdint>

// 1. Color lookups (Index 0 is ALWAYS treated as transparent/void)
inline constexpr uint32_t GLOBAL_PALETTE[256] = {
    0x00000000, // Index 0: Transparent
    0xFF0000FF, // Index 1: Red
    0x00FF00FF, // Index 2: Green
    0x0000FFFF, // Index 3: Blue
};

// 2. Pure RLE Pair sequence: [Pixel Run Length (1-255), Palette Index]
// Example: A 4x4 block of 16 green pixels (Index 2)
inline constexpr uint16_t SPRITE_PLAYER_COMPRESSED_SIZE = 2;
inline constexpr uint8_t SPRITE_PLAYER[2] = {
    16, 2
};
```

#### 2. Manual Font Data Layout (`src/assets/Fonts.h`)

Bitmapped fonts store row-level bitmasks (where bit 15 = leftmost pixel column, bit 0 = rightmost):

```cpp
#pragma once
#include <cstdint>
#include "core/Font.h"

namespace Assets {
    namespace Fonts {
        inline constexpr FontData MY_CUSTOM_FONT = {
            .size = 16,      // Glyphs are 16x16 pixels
            .spacing = 10,   // Cursor x-advance step
            .data = {
                // Style A: Hexadecimal bitmasks
                //  (letter 'A' ASCII 65)
                { 0x3C00, 0x6600, 0x6600, 0x7E00, 0x6600, 0x6600, 0x0000, 0x0000 },

                // Style B: Binary literal bitmasks (what the Crystal packer generates)
                //  (letter 'A' ASCII 65)
                {
                    0b00111100'00000000,
                    0b01100110'00000000,
                    0b01100110'00000000,
                    0b01111110'00000000,
                    0b01100110'00000000,
                    0b01100110'00000000,
                    0b00000000'00000000,
                    0b00000000'00000000
                }
            }
        };
    }
}

```

#### ⚠️ Protecting Custom Asset Headers

* **Do not place raw source files in `assets/fonts/`, `assets/images/`, or `assets/music/**` if you are writing those corresponding C++ headers by hand. The packer only updates a category if matching source files exist in its respective `assets/` subfolder.
* **Safe Cleaning:** Your `make clean-assets` command only clears the internal tracking stamps (`build/.*.stamp`), meaning your hand-crafted headers inside `src/assets/` are fully protected from being wiped out.
* If you write all assets manually and want to disable automated packing completely, you can remove `assets` as a dependency from the `build` target in your `Makefile`.

---

## 📦 Build & Run Usage

### Automated Asset Packing

Force a manual asset repack across all categories:

```bash
make assets
```

Or target individual pipelines if needed:

```bash
make fonts   # Repack fonts only
make images  # Repack images only
make music   # Repack music only
```

### Build (Debug)

Compiles debug build using parallel jobs (automatically triggers asset checks):

```bash
make build
```

### Run (Debug)

Launches compiled binary:

```bash
make run
```

### Full Cycle (Default)

Builds assets, compiles, and launches the game:

```bash
make
```

### Release Build (Stripped Binary)

Compiles optimized Release build:

```bash
make release
```

### Resetting CMake Configuration

If you add/remove C++ files or modify `CMakeLists.txt`, reset the build configuration without wiping built dependency objects:

```bash
make config    # Re-evaluates CMakeLists.txt
make reconfig  # Forces CMake cache refresh (--fresh)
make reset     # Performs clean + fresh CMake configuration
```

### Cleaning Output

```bash
make clean        # Wipes build output directory (build/Debug or build/Release)
make clean-assets # Removes internal asset tracking stamps (preserves manual src/assets/ files)
```

### Output Location

* **Debug builds:** `build/Debug/mini_game`
* **Release builds:** `build/Release/mini_game`

For game jam submissions, grab the compiled release binary at `build/Release/mini_game`!

---

## 🤝 Contributing

1. Fork it (<https://github.com/mswieboda/mini_game/fork>)
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request

## 👤 Contributors

- [Matt Swieboda](https://github.com/mswieboda) - creator and maintainer
