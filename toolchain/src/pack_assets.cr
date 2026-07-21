require "process"
require "file"
require "dir"
require "random"

ASEPRITE_CMD = "aseprite"

# Base paths (relative to toolchain directory)
IMAGES_DIR       = "../assets/images"
MUSIC_DIR        = "../assets/music"
BUILD_DIR        = "build"
SRC_ASSETS_DIR   = "../src/assets"
IMAGE_HEADER     = File.join(SRC_ASSETS_DIR, "ImageData.h")
MUSIC_HEADER     = File.join(SRC_ASSETS_DIR, "MusicData.h")

# Ensure required directories exist
[IMAGES_DIR, MUSIC_DIR, BUILD_DIR, SRC_ASSETS_DIR].each do |dir|
  Dir.mkdir_p(dir) unless Dir.exists?(dir)
end

# Sanitize file names to valid C++ identifiers (e.g., "my-song 1.mod" -> "MY_SONG_1")
def sanitize_symbol_name(path : String) : String
  File.basename(path, File.extname(path))
    .upcase
    .gsub(/[^A-Z0-9_]/, "_")
end

# Helper to process a single Aseprite image and return RLE data
def generate_sprite_rle_data(aseprite_path : String, sprite_path : String)
  raw_data_file = File.join(BUILD_DIR, "temp_#{Random.rand(10000)}.bin")
  palette_file = File.join(BUILD_DIR, "temp_#{Random.rand(10000)}.txt")
  sprite_name = sanitize_symbol_name(sprite_path)

  lua_script_path = "src/export_sprite.lua"

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

  # Apply RLE Compression
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

  # Parse Palette
  colors = [] of String
  File.each_line(palette_file) do |line|
    next if line.starts_with?("#") || line.strip.empty?
    next unless line.split.first?.try &.to_i?
    parts = line.split
    if parts.size >= 4
      colors << "0x%02X%02X%02X%02X" % {parts[3].to_i, parts[0].to_i, parts[1].to_i, parts[2].to_i}
    elsif parts.size >= 3
      colors << "0xFF%02X%02X%02X" % {parts[0].to_i, parts[1].to_i, parts[2].to_i}
    end
  end

  File.delete(raw_data_file) if File.exists?(raw_data_file)
  File.delete(palette_file) if File.exists?(palette_file)

  {
    name: sprite_name,
    palette: colors,
    compressed_size: rle_idx,
    c_array_string: rle_bytes[0...rle_idx].map(&.to_s).join(", ")
  }
end

# ==========================================
# 1. PROCESS IMAGES (assets/images/*.aseprite)
# ==========================================
aseprite_files = Dir.glob("#{IMAGES_DIR}/*.aseprite")
processed_sprites = [] of NamedTuple(name: String, palette: Array(String), compressed_size: Int32, c_array_string: String)

aseprite_files.each do |file|
  puts "Processing image asset: #{file}"
  processed_sprites << generate_sprite_rle_data(ASEPRITE_CMD, file)
end

File.open(IMAGE_HEADER, "w") do |file|
  file.puts "#pragma once"
  file.puts "#include <cstdint>\n\n"
  file.puts "namespace Assets::Images {\n\n"

  if processed_sprites.empty?
    file.puts "    inline const uint32_t GLOBAL_PALETTE[256] = {0};\n"
  else
    global_palette = processed_sprites.first[:palette]
    while global_palette.size < 256
      global_palette << "0x00FF00FF"
    end

    file.puts "    inline const uint32_t GLOBAL_PALETTE[256] = {"
    file.puts "        " + global_palette.join(",\n        ")
    file.puts "    };\n\n"

    processed_sprites.each do |sprite|
      symbol_name = sprite[:name].downcase
      file.puts "    inline const uint16_t #{symbol_name}_len = #{sprite[:compressed_size]};"
      file.puts "    inline const uint8_t #{symbol_name}[#{sprite[:compressed_size]}] = {"
      file.puts "        " + sprite[:c_array_string]
      file.puts "    };\n\n"
    end
  end

  file.puts "} // namespace Assets::Images"
end
puts "Image header packed at: #{IMAGE_HEADER}"


# ==========================================
# 2. PROCESS MUSIC (assets/music/*.[mod|xm|s3m|it])
# ==========================================
music_files = Dir.glob("#{MUSIC_DIR}/*.{mod,xm,s3m,it}")

File.open(MUSIC_HEADER, "w") do |file|
  file.puts "#pragma once"
  file.puts "#include <cstdint>\n\n"
  file.puts "namespace Assets::Music {\n\n"

  if music_files.empty?
    puts "No music files found in #{MUSIC_DIR}."
  else
    music_files.each do |music_path|
      puts "Processing music asset: #{music_path}"
      symbol_name = sanitize_symbol_name(music_path).downcase
      music_bytes = File.open(music_path, &.gets_to_end).to_slice

      file.puts "    inline const size_t #{symbol_name}_len = #{music_bytes.size};"
      file.puts "    inline const uint8_t #{symbol_name}[#{music_bytes.size}] = {"

      # Formats bytes nicely into 12 bytes per line to keep files clean & fast to parse
      file.print "        "
      music_bytes.each_with_index do |byte, idx|
        file.print "0x%02X" % byte
        file.print ", " unless idx == music_bytes.size - 1
        if (idx + 1) % 12 == 0 && idx != music_bytes.size - 1
          file.print "\n        "
        end
      end

      file.puts "\n    };\n\n"
    end
  end

  file.puts "} // namespace Assets::Music"
end
puts "Music header packed at: #{MUSIC_HEADER}"

puts "✅ Asset packing complete!"
