-- Global variables (local to the script file)
local initialPosition = FVector(0,0,0) -- Store the starting position
local totalTime = 0.0                 -- Accumulator for time

-- Parameters for the floating motion
local floatAmplitude = 0.5          -- How high and low it floats from the center (in cm)
local floatFrequency = 0.5            -- How many full up/down cycles per second (adjust for speed)

-- Keep physics variables if you might add other movement later,
-- but they won't be used for the basic up/down float.
local velocity = FVector(0,0,0)
local maxSpeed = 5.0
local accelerationRate = 0.1
local dragCoefficient = 1.5

---------------------------------------------------------------------
-- Called when the game starts or the actor is spawned
---------------------------------------------------------------------
function BeginPlay()
    -- Store the initial location to float around it
    initialPosition = self:GetActorLocation()
    totalTime = 0.0 -- Reset time counter

    -- Reset physics variables (optional, good practice)
    velocity = FVector(0,0,0)

    Log("Automatic Floating Initialized. Amplitude: " .. floatAmplitude .. ", Frequency: " .. floatFrequency)
    Log("Floating around initial position: " .. initialPosition:ToString())
end

---------------------------------------------------------------------
-- Called when the game ends or the actor is destroyed
---------------------------------------------------------------------
function EndPlay()
    -- Cleanup if needed
end

---------------------------------------------------------------------
-- Called when this actor overlaps another actor
---------------------------------------------------------------------
function OnOverlap(OtherActor)
    -- Example: Log the name of the overlapping actor
    -- Log("Overlapped with: " .. OtherActor:GetName())
end

---------------------------------------------------------------------
-- Called every frame
-- dt: Delta Time (time elapsed since the last frame in seconds)
---------------------------------------------------------------------
function Tick(dt)

    -- 1) Update total time
    totalTime = totalTime + dt

    -- 2) Calculate vertical offset using a sine wave
    -- math.sin oscillates smoothly between -1 and 1.
    -- We multiply by frequency to control speed and amplitude to control height.
    -- (totalTime * floatFrequency * 2 * math.pi) makes frequency represent cycles per second.
    local verticalOffset = math.sin(totalTime * floatFrequency * 2.0 * 3.14159) * floatAmplitude

    -- 3) Calculate the new position
    -- Start from the initial position and add only the vertical offset.
    local newLoc = FVector(initialPosition.X, initialPosition.Y, initialPosition.Z + verticalOffset)

    -- 4) Set the actor's location directly
    self:SetActorLocation(newLoc)

    -- Optional: Log position for debugging
    -- Log("Time: " .. string.format("%.2f", totalTime) .. ", Offset: " .. string.format("%.2f", verticalOffset) .. ", New Z: " .. string.format("%.2f", newLoc.Z))

    -- == Input and Physics Code (Commented Out/Removed) ==
    -- No input reading needed for automatic float

    -- No acceleration/drag calculations needed for this simple float
    -- If you wanted the float to feel more physical (e.g., slowing down at peaks),
    -- you would calculate a target position and apply forces/velocity changes
    -- towards it, incorporating drag. But for a simple bob, direct setting is easiest.
end

-- Helper function (if not provided by the environment)
-- Assuming FVector has basic components (X, Y, Z) and a ToString() method
-- Assuming self:GetActorLocation(), self:SetActorLocation() exist
-- Assuming math.sin and math.pi (or an approximation) exist