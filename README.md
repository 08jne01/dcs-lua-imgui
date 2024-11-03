# (Super WIP)

Very early prototype of a Lua Only ImGui Library for DCS World.

## Examples

### Creating Window

```lua
-- do at top of file
dofile(LockOn_Options.script_path.."ImGui.lua")
-- Menu Name is button in the bar across the top.
-- Menu Entry name is an entry in that menu.
-- Menus are created automatically as items are assigned to them.
-- Menu Entries are not unique so you can have multiple of the same name, 
-- it may result in strange behaviour though.
ImGui.AddItem("Menu", "Menu Entry Name", function() 
    -- Code goes here
end)
```

### Immediate Drawing

```lua
-- This simply prints Text! to the imgui window
-- Any variable passed to ImGui:Text will automatically
-- be converted to a string using the tostring method.
ImGui:Text("Text!")
```

#### Capturing State

```lua
-- Outside the AddItem code
local some_state = {
    hello = "world"
}

ImGui.AddItem("Menu Name", "Menu Entry Name", function() 
    -- You can capture state.
    -- This will show 
    -- {
    --    hello = "world"
    -- }
    -- Here ImGui.Serialize is a helper function
    -- which converts tables and their children to strings.
    local s = ImGui.Serialize(some_state)
    ImGui:Text(s)
end)
```

#### Tables

```lua
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
```

If you don't have the same number of columns as the header the empty ones will be filled with ```nil```.

### Control Statements

Any ImGui functions which control flow will take a function this is because DCS is multithreaded so LuaImGui has to build a set of commands to send to the Render Thread.

This has the side-effect of requiring that all code within the imgui statements to be executed. So any control flow functions that take a function will execute that function regardless of the state of the control flow.

Most control flow functions share the below model.

```lua
function ImGui:Something(s, f)
    ImGui:BeginSomething(s)
    f()
    ImGui:EndSomething()
end
```

Since it is easy to pass anonymous function around it makes the syntax easy and similar to normal ImGui.

#### Tree

This is the lua version of TreeNode and TreePop.

```lua
-- Openable Menu with Indent - You can recursively combine these to make complex structures.
ImGui:Tree("Some Tree", function() 
    ImGui:Text("Some Hidden Text")
end)
```

#### Header

This is the lua version of CollapsableHeader. This produces an openable menu but unlike Tree there is no indent.

```lua
-- Open-able Menu without Indent.
ImGui:Header("Some Collapsable Header", function() 
    ImGui:Text("Some More Hidden Text")
end)
```

#### TabBar

This produces a menu with multiple tabs where one tab is displayed at a time depending on what the user selects.

```lua
ImGui:TabBar("Some Tabs", function()
    for i=1,5 do
        ImGui:TabItem(string.format("Tab %d", i), function() 
            ImGui:Text(string.format("This is a tab: %d", i))
        end)     
    end
end)
```
