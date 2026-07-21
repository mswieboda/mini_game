module FontExporter
  def self.export(input_file : String) : String
    var_name = File.basename(input_file, File.extname(input_file)).downcase.gsub(/[^a-z0-9_]/, "_")

    # Defaults if not specified in header metadata
    font_size = 16
    font_spacing = 10

    # 128 ASCII characters x maximum 16 rows array
    font_data = Array.new(128) { Array.new(16, 0_u16) }
    current_ascii = -1
    current_row = 0

    File.each_line(input_file) do |line|
      line = line.strip
      next if line.empty?

      # Parse Metadata Header
      if line.starts_with?("#")
        if line =~ /#\s*size:\s*(\d+)/i
          font_size = $1.to_i
        elsif line =~ /#\s*spacing:\s*(\d+)/i
          font_spacing = $1.to_i
        end
        next
      end

      # Parse Char Declaration: Matches "(ASCII 33)", "ASCII 33", "CHAR: '!'", or "33"
      if match = line.match(/\(ASCII\s+(\d+)\)/i) || line.match(/^ASCII\s+(\d+)/i) || line.match(/^(\d+)$/)
        current_ascii = match[1].to_i
        current_row = 0
        next
      elsif line.match(/^CHAR:/i) && (match = line.match(/\(ASCII\s+(\d+)\)/i))
        current_ascii = match[1].to_i
        current_row = 0
        next
      end

      # Parse Glyph Bitmask Rows
      if current_ascii >= 0 && current_ascii < 128 && current_row < 16
        # Converts '#' or '1' into uint16 bitmask integer
        bitmask = line.chars.each_with_index.reduce(0_u16) do |acc, (char, idx)|
          break acc if idx >= 16 # Don't overflow uint16 shift
          (char == '#' || char == '1') ? (acc | (1_u16 << idx)) : acc
        end

        font_data[current_ascii][current_row] = bitmask
        current_row += 1
      end
    end

    # Generate C++ Initializer List
    String.build do |str|
      str << "        inline constexpr FontData #{var_name} = {\n"
      str << "            .size = #{font_size},\n"
      str << "            .spacing = #{font_spacing},\n"
      str << "            .data = {\n"
      font_data.each_with_index do |rows, char_idx|
        str << "                { "
        str << rows.map(&.to_s).join(", ")
        str << " },"
        str << " // ASCII #{char_idx}\n"
      end
      str << "            }\n"
      str << "        };\n\n"
    end
  end
end
