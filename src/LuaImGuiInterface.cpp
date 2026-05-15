#include "include/LuaImGui.h"
#include "ImGuiDisplay.h"

namespace LuaImGui
{
    void Log( const char* s )
    {
        ImGuiDisplay::Log( s );
    }

    void AddItem( const std::string& menu, const std::string& name, std::function<void()> imgui_function )
    {
        ImGuiDisplay::AddImGuiItem( menu, name, std::move( imgui_function ) );
    }

    void RegisterContext( ImGuiSetContextRoutine ctx, ImGuiSetAllocatorRoutine alloc, ImPlotSetContextRoutine plot_ctx )
    {
        ImGuiDisplay::InitializeContext(ctx, alloc, plot_ctx);
    }
    
    void Create( ImGuiSetContextRoutine ctx, ImGuiSetAllocatorRoutine alloc, ImPlotSetContextRoutine plot_ctx )
    {
        ImGuiDisplay::Create();
        RegisterContext(ctx, alloc, plot_ctx);
        ImGuiDisplay::CreateHook();
    }

    void Destroy()
    {
        ImGuiDisplay::Destroy();
    }
}