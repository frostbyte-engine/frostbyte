local coregui = game:GetService("CoreGui")
-- local coregui = gethui()
do
local old = coregui:FindFirstChild("gui")
if old then old:Destroy() end
end

local gui = Instance.new("ScreenGui")
gui.Name = "gui"
gui.Parent = coregui

local main_frame = Instance.new("Frame")
main_frame.Parent = gui

main_frame.Size = UDim2.new(1, -300, 1, -350)
main_frame.Position = UDim2.new(0, 150, 0, 300)

local play_button = Instance.new("TextButton")
play_button.Parent = main_frame

play_button.Size = UDim2.new(0, 220, 0, 100)
play_button.Position = UDim2.new(0.5, -110, 0.5, -50)

play_button.TextSize = 40

local textbox1 = Instance.new("TextBox")
textbox1.Parent = main_frame
textbox1.Name = "BottomBox"

textbox1.Size = UDim2.fromOffset(200, 30)
textbox1.Position = UDim2.new(0.5, -100, 0.5, 80)

textbox1.TextSize = 30

local textbox2 = Instance.new("TextBox")
textbox2.Parent = main_frame
textbox2.Name = "TopBox"

textbox2.Size = UDim2.fromOffset(220, 28)
textbox2.Position = UDim2.new(0.5, -100, 0.5, -80)

textbox2.TextSize = 30

-- main_frame.InputBegan:Connect(function(input)
--     print("began input", input.UserInputType)
-- end)
-- main_frame.InputChanged:Connect(function(input)
--     print("changed input", input.UserInputType)
-- end)
-- main_frame.InputEnded:Connect(function(input)
--     print("ended input", input.UserInputType)
-- end)

-- local function dragify(Frame)
--     local dragToggle = nil
--     local dragInput = nil
--     local dragStart = nil

--     function updateInput(input)
--         local Delta = input.Position - dragStart
--         local Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + Delta.X, startPos.Y.Scale, startPos.Y.Offset + Delta.Y)
--         game:GetService("TweenService"):Create(Frame, TweenInfo.new(.4, "Exponential"), {Position = Position}):Play()
--         -- Frame.Position = Position
--     end

--     Frame.InputBegan:Connect(function(input)
--         if (input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch) then
--             dragToggle = true
--             dragStart = input.Position
--             startPos = Frame.Position
--             input.Changed:Connect(function()
--                 if (input.UserInputState == Enum.UserInputState.End) then
--                     dragToggle = false
--                 end
--             end)
--         end
--     end)

--     Frame.InputChanged:Connect(function(input)
--         if (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
--             dragInput = input
--         end
--     end)

--     game:GetService("UserInputService").InputChanged:Connect(function(input)
--         if (input == dragInput and dragToggle) then
--         -- if (input.UserInputType == dragInput.UserInputType and dragToggle) then
--             updateInput(input)
--         end
--     end)
-- end

-- dragify(main_frame)
