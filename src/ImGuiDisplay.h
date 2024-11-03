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

struct lua_State;

class ImguiDisplay
{

public:

    struct Command
    {
        std::function<bool(bool*)> command;
        int depth = 0;
        bool* control;
    };

    struct MenuItem
    {
        std::function<void( lua_State* L, std::string, bool*)> imgui_function;
        std::string name;
        bool visible;
    };

    struct Menu
    {
        std::vector<MenuItem> items;
    };

    ImguiDisplay();
    ~ImguiDisplay();

    static void RefreshDisplay( lua_State* L );
    static void DisplayHook();
    static void InputHook( UINT msg, WPARAM w_param, LPARAM l_param );

    static void Create()
    {
        if ( ! display.has_value() )
            display.emplace();
    }
    static void Destroy()
    {
        display.reset();
    }

    static void AddImguiItem( const std::string& menu, const std::string& name, std::function<void( lua_State*, std::string, bool*)>&& imgui_function );

    static void Call( lua_State* L, std::function<bool()> function, int depth )
    {
        if ( display )
            display->commands[L].push_back( Command{ [function]( bool* ) { return function(); }, depth, nullptr } );
    }

    static void Call( lua_State* L, std::function<bool(bool*)> function, int depth, bool* control )
    {
        if ( display )
            display->commands[L].push_back( { function, depth, control } );
    }

    static void Error()
    {
        if ( display )
            display->error = true;
    }

private:

    bool error = false;
    static std::optional<ImguiDisplay> display;

    void Refresh( lua_State* L );
    void Display();
    void Input( UINT msg, WPARAM w_param, LPARAM l_param );


    std::mutex command_mtx;
    std::unordered_map<lua_State*, std::vector<Command>> commands;
    std::map<std::string, Menu> menus;
};
