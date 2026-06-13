local a = Drawing.new("Circle")
a.Filled = true
a.Thickness = 3
a.Radius = 100
a.Visible = true
a.Position = Vector2.new(150, 150)

local el = 0
game:GetService("RunService").RenderStepped:Connect(function(dt)
el += dt
a.Radius = 60 + 40 * math.sin(el)
a.Color = Color3.fromHSV(el / 10, 1, 1)
a.Transparency = el % 1
end)