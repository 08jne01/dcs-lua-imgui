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
    static void SetupHook();

    static void CreateHook();

    static void AddImguiItem( const std::string& menu, const std::string& name, std::function<void( lua_State*, std::string, bool*)>&& imgui_function );

    static void Call( lua_State* L, std::function<bool()> function, int depth )
    {
        display.commands[L].push_back( Command{ [function]( bool* ) { return function(); }, depth, nullptr } );
    }

    static void Call( lua_State* L, std::function<bool(bool*)> function, int depth, bool* control )
    {
        display.commands[L].push_back( { function, depth, control } );
    }

    static void Error()
    {
        display.error = true;
    }

    static void Log( const char* s )
    {
        std::unique_lock lock( display.command_mtx );
        display.console.Add( "%s\n", s );
    }

    static void MenuBar( bool state ) { display.hidden = ! state; }

private:

    bool style_editor_open = false;
    bool console_open = false;
    utils::Log console;

    std::atomic<bool> hidden = false;
    bool hook_created = false;
    bool error = false;
    static ImguiDisplay display;

    void Refresh( lua_State* L );
    void Display();
    void Input( UINT msg, WPARAM w_param, LPARAM l_param );

    // Only accessed from main thread - copied to completed_commands
    std::unordered_map<lua_State*, std::vector<Command>> commands;


    std::mutex command_mtx;
    std::unordered_map<lua_State*, std::vector<Command>> completed_commands;
    std::map<std::string, Menu> menus;
};
