# mini_game

Boilerplate project for a C++ `minifb` game, via macOS or Linux

## Installation

### Dependencies:

```
git submodule update --init --recursive
```

#### Linux specific

```
sudo apt update
sudo apt install build-essential cmake libx11-dev libgl1-mesa-dev
```

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

An array of up to `256` `uint32_t` hex-values representing your color index lookup.

An array of pairs `[count, color_index]`` tracking your compressed RLE image byte stream.

#### Manual Data Interop Layout

Simply bypass the automatic asset compilation setup, create src/engine/assets.h yourself, and populate it manually matching this exact blueprint:

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

## Usage


Build and run via `make` and `Makefile`:

### Assets (with Aseprite - OPTIONAL)

```bash
make assets
```

### Build

```bash
make build
```

### Run

```bash
make run
```

### Build & Run

```bash
make build run
```

or

```bash
make
```

for short


(Probably will change in the future to include debug/release modes, and `make` to just build & run debug mode.)

## Contributing

1. Fork it (<https://github.com/mswieboda/mini_game/fork>)
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request

## Contributors

- [Matt Swieboda](https://github.com/mswieboda) - creator and maintainer
