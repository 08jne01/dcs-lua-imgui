#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <Windows.h>
#include <optional>
#include <future>
#include "ThreadSafeQueue.h"
#include "ImGuiLog.h"
#include "include/LuaImGui.h"

struct lua_State;

class ImGuiDisplay
{

public:

    using SetupHookRoutine = void ( * )( );

    static void Create()
    {
        if ( display.has_value() )
            return;

        display.emplace();
    }

    static void Destroy()
    {
        display.reset();
    }

    // For C++
    struct MenuItem
    {
        std::function<void()> imgui_function;
        std::string name;
        bool visible;
    };

    struct Menu
    {
        std::vector<MenuItem> items;
    };

    // For Lua
    struct Command
    {
        std::function<bool(bool*)> command;
        int depth = 0;
        bool* control;
    };

    struct LuaMenuItem
    {
        std::function<void( lua_State* L, std::string, bool*)> imgui_function;
        std::string name;
        bool visible;
    };

    struct LuaMenu
    {
        std::vector<LuaMenuItem> items;
    };

    ImGuiDisplay() = default;
    ~ImGuiDisplay();

    static void RefreshDisplay( lua_State* L );
    static void DisplayHook();
    static void SetupHook();

    static void CreateHook();

    static void AddImGuiItem( const std::string& menu, const std::string& name, std::function<void()> imgui_function );
    static void AddLuaImGuiItem( const std::string& menu, const std::string& name, std::function<void( lua_State*, std::string, bool*)> imgui_function );

    static void Call( lua_State* L, std::function<bool()> function, int depth )
    {
        if ( display )
            display->commands[L].push_back( Command{ [function]( bool* ) { return function(); }, depth, nullptr } );
    }

    static void Call( lua_State* L, std::function<bool(bool*)> function, int depth, bool* control )
    {
        display->commands[L].push_back( { function, depth, control } );
    }

    static void Error()
    {
        if ( display )
        display->error = true;
    }

    static void Log( const char* s )
    {
        if ( display )
        {
            std::unique_lock lock( display->command_mtx );
            display->console.Add( "%s\n", s );
        }
    }

    static void MenuBar( bool state ) 
    { 
        if ( display )
            display->hidden = ! state; 
    }

    void InitializeContextFunctions();

    static void InitializeContext(
        LuaImGui::ImGuiSetContextRoutine ctx,
        LuaImGui::ImGuiSetAllocatorRoutine alloc,
        LuaImGui::ImPlotSetContextRoutine plot_ctx
    )
    {
        if ( ! display.has_value() )
            return;

        display->initialize_remote_context = true;
        display->ctx = ctx;
        display->alloc = alloc;
        display->plot_ctx = plot_ctx;
        
    }

private:

    // C++ context in another dll
    LuaImGui::ImGuiSetContextRoutine ctx = nullptr;
    LuaImGui::ImGuiSetAllocatorRoutine alloc = nullptr;
    LuaImGui::ImPlotSetContextRoutine plot_ctx = nullptr;
    bool initialize_remote_context = false;

    bool style_editor_open = false;
    bool console_open = false;
    utils::Log console;

    std::atomic<bool> hidden = false;
    bool hook_created = false;
    bool error = false;
    static std::optional<ImGuiDisplay> display;

    void DrawCppImGui();
    void Refresh( lua_State* L );
    void Display();

    // Only accessed from main thread - copied to completed_commands
    std::unordered_map<lua_State*, std::vector<Command>> commands;


    std::mutex command_mtx;
    std::unordered_map<lua_State*, std::vector<Command>> completed_commands;
    std::map<std::string, LuaMenu> lua_menus;
    std::map<std::string, Menu> menus;
};
