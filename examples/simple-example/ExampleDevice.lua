local dev = GetSelf()

-- You need to copy the ImGui.lua from lua/ into your Cockpit/Scripts
dofile(LockOn_Options.script_path.."ImGui.lua")

local some_state = {
    hello = "world"
}

-- Menu Name is button in the bar across the top.
-- Menu Entry name is an entry in that menu.
-- Menus are created automatically as items are assigned to them.
-- Menu Entries are not unique so you can have multiple of the same name, 
-- it may result in strange behaviour though.
ImGui.AddItem("Menu Name", "Menu Entry Name", function() 

    -- This simply prints Text! to the imgui window
    -- Any variable passed to ImGui:Text will automatically
    -- be converted to a string using the tostring method.
    ImGui:Text("Text!")

    -- You can capture state.
    -- This will show 
    -- {
    --    hello = "world"
    -- }
    -- Here ImGui.Serialize is a helper function
    -- which converts tables and their children to strings.
    ImGui:Text(ImGui.Serialize(some_state))


    -- You can write Tables (not lua tables) to organize your data.
    -- The first row is the header this determins the number of columns
    -- for the rest of the table, so be sure to make sure it is at least
    -- more than the other rows.
    ImGui:Table({
        { "Name", "Speed (kts)", "Mass (kg)" }
        { "A-4E", 585, 4469 },
        { "F-100D", 803, 9525 },
        { "Sopwith Camel" }, -- You don't have to have the same number of Columns
    })

    -- If you don't have the same number of columns as the header the empty ones
    -- will be filled with nil.



    -- Any ImGui functions which control flow will take a function
    -- This is because DCS is multithreaded so LuaImGui has to build
    -- a set of commands to send to the Render Thread.

    -- It's very important to realise, all the code is executed. This is again
    -- due to the multithreaded nature of DCS. So even though it looks like a statement
    -- might not execute child code it will. It will simply select which code actually gets
    -- drawn on the window.

    -- Luckily in lua it's fairly easy to pass anonymous functions around which makes
    -- the syntax easier to read.

    -- Open-able Menu with Indent - You can recursively combine these to make complex structures.
    ImGui:Tree("Some Tree", function() 
        ImGui:Text("Some Hidden Text")
    end)

    -- Open-able Menu without Indent.
    ImGui:Header("Some Collapsable Header", function() 
        ImGui:Text("Some More Hidden Text")
    end)

    --This produces a menu with multiple tabs where one tab is displayed at a time 
    -- depending on what the user selects.
    ImGui:TabBar("Some Tabs", function()
        for i=1,5 do
            ImGui:TabItem(string.format("Tab %d", i), function() 
                ImGui:Text(string.format("This is a tab: %d", i))
            end)     
        end
    end)

end)

-- Just for illustration that this is a normal device.
function post_initialize()
end

-- Just for illustration that this is a normal device.
function SetCommand(command,value)
end

-- Just for illustration that this is a normal device.
function update()
end

need_to_be_closed = false -- close lua state after initialization