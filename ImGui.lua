-- MIT License

-- Copyright (c) 2024 Joshua Nelson

-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:

-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.

-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.

lua_imgui_dll_path = lua_imgui_dll_path or LockOn_Options.script_path.."LuaImGui"
lua_imgui_lua_path = lua_imgui_lua_path or LockOn_Options.script_path .. "LuaImGui"

function require_imgui()
    package.cpath = package.cpath .. ";" .. lua_imgui_dll_path .. "\\?.dll"
    success, result = pcall(require, 'LuaImGui')
    if not success then
        print_message_to_user("LuaImGui failed to load dll - See Log")
    end
    return result
end

function require_imgui_stubs()
    package.path = package.path .. ";" .. lua_imgui_lua_path .. "\\?.lua"
    return require("ImGuiStubs")
end

-- Set this global variable to disable imgui
if ( imgui_disabled ) then
    ImGui = require_imgui_stubs()
else
    ImGui = require_imgui()
end

function ImGui:Window(name, f)
    self:Begin(name)
    f()
    self:End()
end

function ImGui:Tree(name, f)
    self:TreeNode(name)
    f()
    self:TreePop()
end

function ImGui:Header(name, f)
    self:CollapsingHeader(name)
    f()
    self:Pop() -- Since Collapsing Header doesn't have a end function
end

function ImGui:TabBar(name, f)
    self:BeginTabBar(name)
    f()
    self:EndTabBar()
end

function ImGui:TabItem(name, f)
    self:BeginTabItem(name)
    f()
    self:EndTabItem()
end

function ImGui:Table(t)

    if type(t) ~= 'table' then
        return
    end

    if type(t[1]) ~= 'table' then
        return
    end

    local columns = #t[1]

    self:Columns(columns)
    for row_i,row in ipairs(t) do
        if type(row) == 'table' then
            for col=1,columns do
                self:Text(tostring(row[col]))
                self:NextColumn()
            end
        end
    end
    self:Columns(1)
end

function ImGui:Plot(plot_name, x_axis_name, y_axis_name, width, f)
    ImGui:BeginPlot(plot_name, x_axis_name, y_axis_name, width)
    f()
    ImGui:EndPlot()
end


function ImGui.Serialize(t, depth, seen)
    
    if t == ImGui then
        error("Did you call ImGui:Serialize? Should be ImGui.Serialize")
    end

    if t == nil then
        return "nil"
    end

    seen = seen or {}

    if seen[t] ~= nil then
        return tostring(t)
    end

    seen[t] = true

    depth = depth or 0

    if depth > 100 then
        return "..."
    end

    local space = string.rep(' ', depth * 4)
    local next_space = string.rep(' ', (depth + 1) * 4)

    local output = "{\n"

    local i_strings = {}
    for i, v in ipairs(t) do
        if type(v) == 'table' then
            table.insert(i_strings, next_space..ImGui.Serialize(v, depth + 1, seen))
        elseif type(v) == 'string' then
            table.insert(i_strings, next_space..i..string.format("\"%s\"", v))
        else
            table.insert(i_strings, next_space..tostring(v))
        end
    end

    


    local strings = {}
    for i, v in pairs(t) do

        if i_strings[i] == nil then -- check it hasn't been included in i_strings already
            if type(v) == 'table' then
                table.insert(strings, next_space..i.." = "..ImGui.Serialize(v, depth + 1, seen))
            elseif type(v) == 'string' then
                table.insert(strings, next_space..i..string.format(" = \"%s\"", v))
            else
                table.insert(strings, next_space..i.." = "..tostring(v))
            end
        end
    end

    local mt = getmetatable(t)
    if mt ~= nil then
        table.insert( strings, next_space..string.format("metatable(%s) = ", tostring(getmetatable(t)))..ImGui.Serialize(getmetatable(t), depth + 1, seen))
    end

    strings = table.concat(strings, ',\n')
    
    if #i_strings > 0 then
        i_strings = table.concat(i_strings, ',\n')
        strings = table.concat({i_strings, strings}, ',\n')
    end
    output = output..strings..'\n'

    output = output..space.."}"
    return output
end