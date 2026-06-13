if shared.backgroundanimation then
    shared.backgroundanimation:Disconnect()
    shared.backgroundanimation = nil
    return
end

local viewport = workspace.CurrentCamera.ViewportSize

local BUFFER_BYTES = 16
local BUFFER_SIZE = BUFFER_BYTES * 2000
local buf = buffer.create(BUFFER_SIZE)

local ENTITY_SIZE = Vector2.new(50, 50)

for i = 0, BUFFER_SIZE - 1, BUFFER_BYTES do
    -- buffer.writei32(buf, i, math.random(-10, viewport.X))
    buffer.writei32(buf, i, math.random(0, viewport.X / ENTITY_SIZE.X + 1) * ENTITY_SIZE.X)
    buffer.writei32(buf, i + 4, math.random(0, viewport.Y / ENTITY_SIZE.Y) * ENTITY_SIZE.Y)
    local h = i / BUFFER_SIZE * 255
    local s = 100
    local v = 200
    buffer.writei32(buf, i + 8, bit32.bor(h, bit32.bor(bit32.lshift(s, 8), bit32.lshift(v, 16))))
end

-- local time = tick()
local count = 0
shared.backgroundanimation = DrawingImmediate.GetPaint(0):Connect(function(dt)
    count += 1
    local passed = count > 9
    if passed then
        count = 0
    end
    -- local timenow = tick()
    -- local dt = timenow - time
    -- time = timenow
    for i = 0, BUFFER_SIZE - 1, BUFFER_BYTES do
        local x = buffer.readi32(buf, i)
        local y = buffer.readi32(buf, i + 4)

        local color = buffer.readi32(buf, i + 8)
        local color_h = bit32.band(color, 0xFF) / 255;
        local color_s = bit32.band(bit32.rshift(color, 8), 0xFF) / 255;
        local color_v = bit32.band(bit32.rshift(color, 16), 0xFF) / 255;

        DrawingImmediate.FilledRectangle(Vector2.new(x, y), ENTITY_SIZE, Color3.fromHSV(color_h, color_s, color_v), 1, 0, 1)

        if x >= viewport.X then
            -- buffer.writei32(buf, i, math.random(-100, -50))
            buffer.writei32(buf, i, -45)
        elseif passed then
            -- buffer.writei32(buf, i, x + math.random() + 50.5 * dt)
            -- buffer.writei32(buf, i, x + 1)
            local newx = x + math.abs(x) / viewport.X * 5 + 10
            if newx > viewport.X then newx = -45 end
            buffer.writei32(buf, i, newx)
            -- if i == 0 then print(x, newx) end
        end
    end
end)

