-- ┌─────────────────────────────────────────────────────┐
-- │           Boink Engine - Example Script             │
-- │  Loaded from C++ via window:loadScript()           │
-- └─────────────────────────────────────────────────────┘

-- Called once when the script loads
function onLoad()
    print("Boink script loaded!")

    -- Position camera
    Boink.cameraMoveTo(0, 3, 8)
    Boink.cameraLookAt(0, 0, 0)
    Boink.cameraSetFov(60)

    -- Ground plane (10x10 subdivisions)
    local ground = Boink.addPlane("ground", 10)
    ground:moveTo(0, -1, 0)
    ground:scaleTo(10, 1, 10)
    ground:setColor(1.0, 1.0, 1.0)          -- white tint so texture shows true color
    ground:setTexture("textures/grass.png")  -- place a PNG here, or comment this out
    ground:setTextureTile(5)                 -- repeat texture 5x across the plane

    -- Central spinning cube
    local cube = Boink.addCube("mainCube")
    cube:moveTo(0, 0, 0)
    cube:setColor(1.0, 1.0, 1.0)
    cube:setTexture("scripts/textures/template.jpg")  -- place a PNG here, or comment this out

    -- Red wireframe cube next to it
    local wire = Boink.addCube("wireCube")
    wire:moveTo(2.5, 0, 0)
    wire:setColor(1.0, 0.3, 0.3)
    wire:setWireframe(true)

    -- Triangle above
    local tri = Boink.addTriangle("hat")
    tri:moveTo(0, 1.5, 0)
    tri:scaleTo(1.5, 1.5, 1.5)
    tri:setColor(1.0, 0.85, 0.1)

    -- Small cube cluster in the back
    for i = 1, 4 do
        local c = Boink.addCube("mini_" .. i)
        local angle = (i-1) * (3.14159 * 2 / 4)
        c:moveTo(math.cos(angle) * 4, -0.5, math.sin(angle) * 4)
        c:scaleTo(0.5, 0.5, 0.5)
        c:setColor(math.random(), math.random(), math.random())
    end

    print("Scene setup complete. WASD = move, Arrows = orbit, Q/E = up/down, Esc = quit")
end

local time = 0

-- Called every frame with delta time in seconds
function onUpdate(dt)
    time = time + dt

    -- Spin the main cube
    local cube = Boink.getShape("mainCube")
    if cube then
        cube:rotateTo(time * 0.5, time, 0)
    end

    -- Triangle bobs up and down
    local tri = Boink.getShape("hat")
    if tri then
        tri:moveTo(0, 1.5 + math.sin(time * 2) * 0.3, 0)
        tri:rotateTo(0, time * 2, 0)
    end

    -- Wire cube rotates on another axis
    local wire = Boink.getShape("wireCube")
    if wire then
        wire:rotateTo(time * 0.7, 0, time * 0.4)
    end

    -- Mini cubes spin in orbit
    for i = 1, 4 do
        local c = Boink.getShape("mini_" .. i)
        if c then
            local angle = (i-1) * (3.14159 * 2 / 4) + time * 0.5
            c:moveTo(math.cos(angle) * 4, -0.5 + math.sin(time + i) * 0.3,
                     math.sin(angle) * 4)
            c:rotateTo(time, time * 0.5 * i, 0)
        end
    end
end
