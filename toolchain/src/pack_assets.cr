require "process"
require "file"
require "dir"
require "random"

# --- Main Dynamic Scanning Loop ---
ASEPRITE_CMD  = "aseprite"
ASSETS_DIR    = "../assets"
ASSETS_HEADER = "../src/assets.h"
BUILD_DIR     = "build"

# Ensure our asset and hidden build directories exist
Dir.mkdir(ASSETS_DIR) unless Dir.exists?(ASSETS_DIR)
Dir.mkdir(BUILD_DIR) unless Dir.exists?(BUILD_DIR)

# Helper to process a single file and return its generated C++ code block strings
def generate_sprite_rle_data(aseprite_path : String, sprite_path : String)
  raw_data_file = File.join(BUILD_DIR, "temp_#{Random.rand(10000)}.bin")
  palette_file = File.join(BUILD_DIR, "temp_#{Random.rand(10000)}.txt")
  sprite_name = File.basename(sprite_path, File.extname(sprite_path)).upcase

  lua_script_path = "src/export_sprite.lua"
  err_stream = IO::Memory.new

  # Run Aseprite and inherit the streams so all logs/errors print directly to the terminal
  res = Process.run(aseprite_path, [
    "-b", sprite_path,
    "--script-param", "filename=#{raw_data_file}",
    "--script-param", "palette=#{palette_file}",
    "--script", lua_script_path
  ], output: Process::Redirect::Inherit, error: Process::Redirect::Inherit)

  unless res.success? && File.exists?(raw_data_file) && File.exists?(palette_file)
    STDERR.puts "\n❌ Error: Aseprite export pipeline broken for #{sprite_path}."
    exit(1)
  end

  raw_pixels = File.open(raw_data_file, &.gets_to_end).to_slice

  # 2. Apply RLE
  rle_bytes = Bytes.new(raw_pixels.size * 2)
  rle_idx = 0

  if raw_pixels.size > 0
    current_color = raw_pixels[0]
    run_length = 1

    (1...raw_pixels.size).each do |i|
      pixel = raw_pixels[i]
      if pixel == current_color && run_length < 255
        run_length += 1
      else
        rle_bytes[rle_idx] = run_length.to_u8; rle_idx += 1
        rle_bytes[rle_idx] = current_color;    rle_idx += 1
        current_color = pixel
        run_length = 1
      end
    end
    rle_bytes[rle_idx] = run_length.to_u8; rle_idx += 1
    rle_bytes[rle_idx] = current_color;    rle_idx += 1
  end

  # 3. Parse Local Palette (We extract the first valid palette we find to act as global lookup)
  colors = [] of String
  File.each_line(palette_file) do |line|
    next if line.starts_with?("#") || line.strip.empty?
    next unless line.split.first?.try &.to_i?
    parts = line.split
    if parts.size >= 3
      colors << "0x%02X%02X%02XFF" % {parts[0].to_i, parts[1].to_i, parts[2].to_i}
    end
  end


  # If a crash happens before this point, the files stay in 'build/' for debugging!
  File.delete(raw_data_file) if File.exists?(raw_data_file)
  File.delete(palette_file) if File.exists?(palette_file)

  # Return structural data chunks back to main loop
  {
    name: sprite_name,
    palette: colors,
    compressed_size: rle_idx,
    c_array_string: rle_bytes[0...rle_idx].map(&.to_s).join(", ")
  }
end

unless Dir.exists?(ASSETS_DIR)
  Dir.mkdir(ASSETS_DIR)
end

# Find all matching source entries
aseprite_files = Dir.glob("#{ASSETS_DIR}/*.aseprite")

if aseprite_files.empty?
  puts "No .aseprite files found in #{ASSETS_DIR}. Generating empty fallback assets.h..."
  File.write(ASSETS_HEADER, "#pragma once\nconst uint32_t GLOBAL_PALETTE[256] = {0};\n")
  exit(0)
end

processed_sprites = [] of NamedTuple(name: String, palette: Array(String), compressed_size: Int32, c_array_string: String)

aseprite_files.each do |file|
  puts "Processing target asset: #{file}"
  processed_sprites << generate_sprite_rle_data(ASEPRITE_CMD, file)
end

# Write unified header output file
File.open(ASSETS_HEADER, "w") do |file|
  file.puts "#pragma once"
  file.puts "#include <cstdint>\n\n"

  # Use the palette from the first processed file as the unified global color space
  global_palette = processed_sprites.first[:palette]
  while global_palette.size < 256
    global_palette << "0x000000FF"
  end

  file.puts "const uint32_t GLOBAL_PALETTE[256] = {"
  file.puts "    " + global_palette.join(",\n    ")
  file.puts "};\n\n"

  # Sequentially dump every single separate sprite block!
  processed_sprites.each do |sprite|
    file.puts "const uint16_t SPRITE_#{sprite[:name]}_COMPRESSED_SIZE = #{sprite[:compressed_size]};"
    file.puts "const uint8_t SPRITE_#{sprite[:name]}[#{sprite[:compressed_size]}] = {"
    file.puts "    " + sprite[:c_array_string]
    file.puts "};\n\n"
  end
end

puts "Unified header built successfully at #{ASSETS_HEADER}!"
