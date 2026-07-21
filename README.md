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

## 🛠️ Asset Pipeline Documentation & Troubleshooting

The asset system runs on a **Crystal**-powered asset packing utility (`toolchain/src/pack_assets.cr`) that automatically extracts your design assets, applies Run-Length Encoding (RLE) compression, and generates standard C++ array outputs (`src/assets.h`).

### Option A: Using the Automated Aseprite Setup

The packing script expects the keyword `aseprite` to be mapped to your system command-line space. Because Aseprite packages on macOS are wrapped in an application bundle directory, standard symlinks cause resource initialization failures. Follow this setup:

1. Remove any old symbolic references:
   ```bash
   rm -f ~/.local/bin/aseprite
   ```

2. Create a path-relative wrapper shell file at your local binary directory (e.g., `~/.local/bin/aseprite` or `/usr/local/bin/aseprite` depending on what is tracked in your shell profile's `$PATH` layout):

   `aseprite` file:
   ```bash
   #!/bin/bash
   exec /Applications/Aseprite.app/Contents/MacOS/aseprite "$@"
   ```

3. Mark the script executable:
   ```bash
   chmod +x ~/.local/bin/aseprite
   ```

4. Verify your terminal execution:
   ```bash
   aseprite --version
   ```

### Option B: Alternative Editors (Without Automatic Aseprite Pipeline)

If you do not have Aseprite installed, or prefer to use an alternative editor (like Libresprite, GIMP, or Piskel), you can easily generate data arrays manually!

The C++ core renderer asks for two items inside `src/assets.h` layout:

1. An array of up to `256` `uint32_t` hex-values representing your color index lookup.
2. An array of pairs `[count, color_index]` tracking your compressed RLE image byte stream.

#### Manual Data Interop Layout

Simply bypass the automatic asset compilation setup, create `src/engine/assets.h` yourself, and populate it manually matching this exact blueprint:

```cpp
#pragma once
#include <cstdint>

// 1. Color lookups (Index 0 is ALWAYS treated as transparency/invisible)
const uint32_t GLOBAL_PALETTE[256] = {
    0x00000000, // Index 0: Alpha/Transparent Void
    0xFF0000FF, // Index 1: Pure Red
    0x00FF00FF, // Index 2: Pure Green
    0x0000FFFF, // Index 3: Pure Blue
    // ... pad with 0x000000FF to 256 elements if desired
};

// 2. Pure RLE Pair sequence: [Pixel Run Length (1-255), Palette Index Number]
// Example: A 4x4 block of 16 green pixels would just be: { 16, 2 }
const uint16_t SPRITE_PLAYER_COMPRESSED_SIZE = 2;
const uint8_t SPRITE_PLAYER[2] = {
    16, 2
};
```

If you maintain this format, your software game loop will build flawlessly using the generic workspace `make build` and `make run` pipeline controls, regardless of where or how you built the hex sequences!

---

## 📦 Build & Run Usage

Build and run via `make`:

### Assets (with Aseprite - OPTIONAL)

```bash
make assets
```

### Build (Debug)

```bash
make build
```

### Run (Debug)

```bash
make run
```

### Build & Run (Debug)

```bash
make
```

### Release Mode (Stripped & UPX Compressed)

```bash
# Build release binary
make build-release

# Run release binary
make run-release

# Clean & Build & Run release binary shortcut
make release
```

### Clean Build Output

```bash
make clean
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
