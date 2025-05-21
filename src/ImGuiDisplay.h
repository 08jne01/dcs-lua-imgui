#pragma once
#include <functional>
#include <map>
#include <memory>
#include <variant>
#include <string>
#include <unordered_map>
#include <optional>
#include <mutex>
#include "ImGuiLog.h"
#include "include/LuaImGui.h"

struct lua_State;

class ImGuiDisplay
{
    using LuaData = std::variant<bool,double,int>;

    struct ImGuiResult
    {
        std::string source_identifier;
        LuaData data = false;
    };

    static bool consteval IsDisabled()
    {
#ifdef LUA_IMGUI_DISABLED
        return true;
#else
        return false;
#endif
    }

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

    using CommandType = std::variant<
        std::function<void()>, // immediate
        std::function<bool()>,
        std::function<bool(bool*)> // control flow
    >;

    // For Lua
    struct Command
    {
        bool operator()( const std::function<void()>& command ) const
        {
            command();
            return true;
        }

        bool operator()( const std::function<bool(bool*)>& command ) const
        {
            return command(control);
        }

        bool operator()( const std::function<bool()>& command ) const
        {
            return command();
        }

        // bool operator()(const std::function<bool()>& command) const
        // {
        //     if ( std::holds_alternative<bool>(result->data) )
        //         std::get<bool>(result->data) |= command();
        //     else
        //         result->data = command();
        //     return false;
        // }

        // bool operator()(const std::function<double()>& command) const
        // {
        //     result->data = command();
        //     return false;
        // }

        // bool operator()(const std::function<std::string()>& command) const
        // {
        //     result->data = command();
        //     return false;
        // }

        CommandType command;
        int depth = 0;
        bool* control = nullptr;
        //ImGuiResult* result = nullptr;
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

    template<typename T>
    static void Call(lua_State* L, T&& function, int depth)
    {
        if ( display )
        {
            display->commands[L].emplace_back( std::move(std::function(std::forward<T&&>(function))), depth, nullptr );
        }
    }

    static void Call( lua_State* L, std::function<bool(bool*)> function, int depth, bool* control )
    {
        if ( display )
        {
            display->commands[L].emplace_back( std::move(function), depth, control );
        }
    }

    static ImGuiResult* GetResult( lua_State* L, const char* source_identifier )
    {
        if ( display )
        {
            std::unique_lock lock( display->command_mtx );
            auto& results = display->results[L];
            auto it = std::find_if( results.begin(), results.end(), [source_identifier](const auto& result){
                return result->source_identifier == source_identifier;
            });

            if ( it != results.end() )
                return it->get(); // return last frames data

            auto& ptr = results.emplace_back( std::make_unique<ImGuiResult>(source_identifier, false) );
            return ptr.get();
        }
        return nullptr;
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

        std::unique_lock lock(display->command_mtx);
        display->contexts.emplace_back(ctx, alloc, plot_ctx);
    }

private:

    struct ImGuiCtx
    {
        LuaImGui::ImGuiSetContextRoutine ctx = nullptr;
        LuaImGui::ImGuiSetAllocatorRoutine alloc = nullptr;
        LuaImGui::ImPlotSetContextRoutine plot_ctx = nullptr;   
    };

    std::vector<ImGuiCtx> contexts;

    // C++ context in another dll
    
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

    std::unordered_map<lua_State*, std::vector<std::unique_ptr<ImGuiResult>>> results;

    std::mutex command_mtx;
    std::unordered_map<lua_State*, std::vector<Command>> completed_commands;
    std::map<std::string, LuaMenu> lua_menus;
    std::map<std::string, Menu> menus;
};
