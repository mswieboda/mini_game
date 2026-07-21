# toolchain/src/export/image.cr

require "process"
require "file"
require "dir"
require "random"

module ImageExporter
  ASEPRITE_CMD = "aseprite"
  BUILD_DIR    = "build"

  # __DIR__ points to "toolchain/src/export"
  # This resolves cleanly to the full path of "aseprite_to_bytes.lua"
  LUA_SCRIPT   = File.expand_path("aseprite_to_bytes.lua", __DIR__)

  struct SpriteData
    property name : String
    property palette : Array(String)
    property compressed_size : Int32
    property c_array_string : String

    def initialize(@name, @palette, @compressed_size, @c_array_string)
    end
  end

  def self.process_sprite(sprite_path : String) : SpriteData
    Dir.mkdir_p(BUILD_DIR) unless Dir.exists?(BUILD_DIR)

    raw_data_file = File.join(BUILD_DIR, "temp_#{Random.rand(10000)}.bin")
    palette_file  = File.join(BUILD_DIR, "temp_#{Random.rand(10000)}.txt")

    base_name   = File.basename(sprite_path, File.extname(sprite_path))
    symbol_name = base_name.downcase.gsub(/[^a-z0-9_]/, "_")

    res = Process.run(
      ASEPRITE_CMD,
      [
        "-b", sprite_path,
        "--script-param", "filename=#{raw_data_file}",
        "--script-param", "palette=#{palette_file}",
        "--script", LUA_SCRIPT
      ],
      output: Process::Redirect::Inherit,
      error: Process::Redirect::Inherit
    )

    unless res.success? && File.exists?(raw_data_file) && File.exists?(palette_file)
      STDERR.puts "\n❌ Error: Aseprite export pipeline failed for #{sprite_path}."
      STDERR.puts "   Lua script target: #{LUA_SCRIPT}"
      exit(1)
    end

    raw_pixels = File.open(raw_data_file, &.gets_to_end).to_slice

    # RLE Compression
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

    SpriteData.new(
      name: symbol_name,
      palette: colors,
      compressed_size: rle_idx,
      c_array_string: rle_bytes[0...rle_idx].map(&.to_s).join(", ")
    )
  end
end
