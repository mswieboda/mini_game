local sprite = app.activeSprite
if not sprite then
    io.stderr:write("Error: No active sprite found.\n")
    os.exit(1)
end

local target_filename = app.params["filename"]
local palette_filename = app.params["palette"]

if not target_filename or not palette_filename then
    io.stderr:write("Error: Missing parameters.\n")
    os.exit(1)
end

-- 1. DRAW SPRITE TO A FRESH RGB IMAGE BUFFER
-- This automatically flattens all visible layers into true RGBA values!
local img = Image(sprite.width, sprite.height, ColorMode.RGB)
img:drawSprite(sprite, 1) -- Draws frame 1 (1-based index in Lua)

local pal = sprite.palettes[1]

-- Cache the palette colors in Lua to look up indices instantly by true color matching
local color_to_index = {}
if pal then
    for i = 0, #pal - 1 do
        local c = pal:getColor(i)
        -- Create a unique color key string "R,G,B,A"
        local key = string.format("%d,%d,%d,%d", c.red, c.green, c.blue, c.alpha)
        if color_to_index[key] == nil then
            color_to_index[key] = i
        end
    end
end

-- 2. EXPORT RAW PIXEL DATA
local file = io.open(target_filename, "wb")
if not file then
    io.stderr:write("Error: Could not open output file.\n")
    os.exit(1)
end

for y = 0, sprite.height - 1 do
    for x = 0, sprite.width - 1 do
        local color_byte = 0

        if x < img.width and y < img.height then
            local raw_pixel = img:getPixel(x, y)

            -- Extract true RGBA components from the pure RGB image buffer
            local r = app.pixelColor.rgbaR(raw_pixel)
            local g = app.pixelColor.rgbaG(raw_pixel)
            local b = app.pixelColor.rgbaB(raw_pixel)
            local a = app.pixelColor.rgbaA(raw_pixel)

            -- If pixel is fully transparent, treat it as color index 0
            if a == 0 then
                color_byte = 0
            else
                local key = string.format("%d,%d,%d,%d", r, g, b, a)
                color_byte = color_to_index[key] or 0
            end
        end

        file:write(string.char(color_byte))
    end
end
file:close()

-- 3. EXPORT RAW PALETTE TEXT
local pal_file = io.open(palette_filename, "w")
if pal_file then
    if pal then
        for i = 0, #pal - 1 do
            local color = pal:getColor(i)
            pal_file:write(string.format("%d %d %d\n", color.red, color.green, color.blue))
        end
    end
    pal_file:close()
end
