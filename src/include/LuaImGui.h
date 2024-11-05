#pragma once
#include <string>
#include <functional>
#include "LuaImGuiApi.h"

#include "imgui.h"
#include "implot.h"

namespace LuaImGui
{
    //// ImGui Interface
    LUA_IMGUI_API void AddItem( const std::string& menu, const std::string& name, std::function<void()> imgui_function );

    // Do not call these in dllmain
    using ImGuiSetContextRoutine = void ( * )( ImGuiContext* );
    using ImGuiSetAllocatorRoutine = void ( * )( ImGuiMemAllocFunc, ImGuiMemFreeFunc, void* );
    using ImPlotSetContextRoutine = void ( * )( ImPlotContext* );

    // You need to pass in the functions
    // ImGui::SetCurrentContext
    // ImGui::SetAllocatorFunctions
    // ImPlot::SetCurrentContext
    LUA_IMGUI_API void Create( ImGuiSetContextRoutine ctx, ImGuiSetAllocatorRoutine alloc, ImPlotSetContextRoutine plot_ctx );
    LUA_IMGUI_API void Destroy();
}
