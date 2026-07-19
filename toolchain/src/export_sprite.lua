-- toolchain/src/export_sprite.lua
local sprite = app.activeSprite
if not sprite then
    io.stderr:write("Error: No active sprite found.\n")
    os.exit(1)
end

local target_filename = app.params["filename"]
local palette_filename = app.params["palette"]

if not target_filename or not palette_filename then
    io.stderr:write("Error: Missing parameters. Requires both 'filename' and 'palette'.\n")
    os.exit(1)
end

-- Safely search and define the first valid 'cel' containing an actual image block
local cel = nil
for _, c in ipairs(sprite.cels) do
    if c.image then
        cel = c
        break
    end
end

if not cel then
    io.stderr:write("Error: No valid image cell data found in this sprite file.\n")
    os.exit(1)
end

-- 1. EXPORT RAW PIXEL DATA
local file = io.open(target_filename, "wb")
if not file then
    io.stderr:write("Error: Could not open output file: " .. target_filename .. "\n")
    os.exit(1)
end

local img = cel.image
for y = 0, sprite.height - 1 do
    for x = 0, sprite.width - 1 do
        local color_byte = 0

        -- Safe check: Is our coordinate inside the drawn bounding box of the cel?
        if x >= cel.position.x and x < cel.position.x + img.width and
           y >= cel.position.y and y < cel.position.y + img.height then

            local raw_pixel = img:getPixel(x - cel.position.x, y - cel.position.y)

            -- Downcast Aseprite's internal 32-bit color integer safely down to a single byte (0-255)
            if sprite.colorMode == ColorMode.INDEXED then
                color_byte = app.pixelColor.indexedI(raw_pixel)
            else
                -- Fallback for standard RGB images: Extract the Red channel value (0-255)
                color_byte = app.pixelColor.rgbaR(raw_pixel)
            end
        end

        -- Double guard to ensure the byte is strictly bounded to 0-255 before passing to string.char
        if color_byte < 0 or color_byte > 255 then
            color_byte = 0
        end

        file:write(string.char(color_byte))
    end
end
file:close()

-- 2. EXPORT RAW PALETTE TEXT
local pal_file = io.open(palette_filename, "w")
if not pal_file then
    io.stderr:write("Error: Could not open palette file: " .. palette_filename .. "\n")
    os.exit(1)
end

local pal = sprite.palettes[1]
if pal then
    for i = 0, #pal - 1 do
        local color = pal:getColor(i)
        pal_file:write(string.format("%d %d %d\n", color.red, color.green, color.blue))
    end
end
pal_file:close()
