local ImGui = {}

function ImGui:Stub(f)
    self[f] = function() end
end

-- C Defined
ImGui:Stub("BeginTabBar")
ImGui:Stub("EndTabBar")
ImGui:Stub("BeginTabItem")
ImGui:Stub("EndTabItem")
ImGui:Stub("TreeNode")
ImGui:Stub("TreePop")
ImGui:Stub("CollapsingHeader")
ImGui:Stub("Pop")
ImGui:Stub("Begin")
ImGui:Stub("End")
ImGui:Stub("Text")
ImGui:Stub("Columns")
ImGui:Stub("NextColumn")
ImGui:Stub("AddItem")
ImGui:Stub("Refresh")
ImGui:Stub("MenuBar")

return ImGui