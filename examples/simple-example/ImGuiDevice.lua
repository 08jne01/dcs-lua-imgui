-- This device exists solely to provide destroy the underlying ImGui objects when the plane is closed.
local dev = GetSelf()
dofile(LockOn_Options.script_path.."ImGui.lua")

-- Just for illustration that this is a normal device.
function post_initialize()
end

-- Just for illustration that this is a normal device.
function SetCommand(command,value)
end

-- Destroy the ImGui - This only needs to happen in one place.
function release()
    ImGui:Destroy()
end

need_to_be_closed = false -- close lua state after initialization