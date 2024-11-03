function require_imgui()
  package.cpath = package.cpath..";"..LockOn_Options.script_path.."LuaImGui\\?.dll"
  success,result = pcall(require,'LuaImGui')
  return result
end

ImGui = require_imgui()

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


function ImGui.Serialize(t, depth, seen)
    
    if t == nil then
        return "nil"
    end

    seen = seen or {}

    if seen[t] ~= nil then
        return tostring(t)
    end

    seen[t] = true

    depth = depth or 0

    if depth > 10 then
        return "..."
    end

    local space = string.rep(' ', depth * 4)
    local next_space = string.rep(' ', (depth + 1) * 4)

    local output = "{\n"

    local strings = {}
    for i,v in pairs(t) do
        if type(v) == 'table' then
            table.insert(strings, next_space..i.." = "..ImGui.Serialize(t, depth + 1, seen))
        else
            table.insert(strings, next_space..i.." = "..tostring(v))
        end
    end

    local mt = getmetatable(t)
    if mt ~= nil then
        table.insert( strings, next_space..string.format("metatable(%s) = ", tostring(getmetatable(t)))..ImGui.Serialize(getmetatable(t), depth + 1, seen))
    end
    -- for i,v in pairs(getmetatable(t)) do
    --     if type(v) == 'table' then
    --         table.insert(strings, i.." = "..next_space..Serialize(t, depth + 1, seen))
    --     else
    --         table.insert(strings, i.." = "..next_space..tostring(v))
    --     end
    -- end

    output = output..table.concat(strings, ',\n')..'\n'

    output = output..space.."}"
    return output
end