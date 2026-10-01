if shared.circlebg then
    shared.circlebg[1]:Disconnect()
    shared.circlebg[2]:Destroy()
    shared.circlebg = nil
    return
end

local circle = Drawing.new("Circle")

circle.Radius = 5
circle.Transparency = 33/255
circle.Visible = true

local max = workspace.CurrentCamera.ViewportSize.X * (math.pi / 2)

local elapsed = -1
shared.circlebg = {game:GetService("RunService").RenderStepped:Connect(function(dt)
    elapsed += dt
    local percent = (math.sin(elapsed * 1.5) + 1) / 2

    circle.Radius = percent * max
end), circle}
