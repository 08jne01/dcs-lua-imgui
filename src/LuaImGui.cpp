#include "LuaImGui.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

#include <future>
#include "imgui.h"

#include "ImGuiDisplay.h"

//#define REGISTER_FUNCTION( L, function ) \
//    lua_pushcfunction(L, l_##function ); \
//    lua_setfield( L, -2, #function )
//
//#define REGISTER_FUNCTION_NAMED(L, function, name) \
//    lua_pushcfunction(L, function) \
//    lua_setfield(L, -2, name)

#define REGISTER_FUNCTION(function) { #function, l_##function }

#define CONTROL_PUSH( function ) l_ControlFlowPush<[]( const char* c ) { return function(c); }>
#define CONTROL_POP( function ) l_ControlFlowPop<[]() { return function(); }>

#define REGISTER_CONTROL_PUSH(function) { #function, CONTROL_PUSH(ImGui::##function)}
#define REGISTER_CONTROL_POP(function) { #function, CONTROL_POP(ImGui::##function)}

namespace LuaImGui
{
    void Log( lua_State* L, int idx )
    {   
        lua_getglobal( L, "log" );
        int log = lua_gettop( L );
        lua_getfield( L, log, "write" );

        //lua_getglobal( L, "tostring" );
        //lua_pushvalue( L, idx );
        //lua_call( L, 1, 1 );
        lua_pushstring( L, "LuaImGui" );
        lua_getfield( L, log, "ERROR" );

        lua_getglobal( L, "tostring" );
        lua_pushvalue( L, idx );
        lua_call( L, 1, 1 );


        lua_call( L, 3, 0 );
        lua_pop( L, 1 );
    }

    void Print( lua_State* L, int idx )
    {
        lua_getglobal( L, "print_message_to_user" );
        lua_getglobal( L, "tostring" );
        lua_pushvalue( L, idx );
        lua_call( L, 1, 1 );

        //const char* str = lua_tolstring( L, idx, nullptr );
        lua_call( L, 1, 0 );
    }

    struct LuaFunction
    {
        LuaFunction( lua_State* L_in, int stack ) :
            L( L_in )
        {
            lua_pushvalue( L, stack );
            function_ref = luaL_ref( L, LUA_REGISTRYINDEX );
        }
        lua_State* L;
        int function_ref; // We don't delete it because the lua state gets blatted before this

        void Call() const
        {
            lua_rawgeti( L, LUA_REGISTRYINDEX, function_ref );
            int error = lua_pcall( L, 0, 0, 0 );
            if ( error )
            {
                Log( L, lua_gettop(L) );
                lua_pop( L, 1 );
                ImguiDisplay::Error();
            }
        }
    };

    int Frame( lua_State* L )
    {
        lua_getfield( L, 1, "depth" );
        //Print( L, -1 );
        int depth = lua_tointeger( L, -1 );
        lua_pop( L, 1 );
        return depth;
    }

    int PushFrame( lua_State* L )
    {
        int depth = Frame( L );
        lua_pushinteger( L, depth + 1 );
        lua_setfield( L, 1, "depth" );
        return depth;
    }

    int PopFrame( lua_State* L )
    {
        int depth = Frame( L );
        lua_pushinteger( L, depth - 1 );
        lua_setfield( L, 1, "depth" );
        return depth;
    }

    int l_Pop( lua_State* L )
    {
        PopFrame( L );
        return 0;
    }

    int l_Refresh( lua_State* L )
    {
        ImguiDisplay::RefreshDisplay( L );
        return 0;
    }

    int l_AddItem( lua_State* L )
    {
        if ( lua_isfunction( L, 3 ) )
        {
            const char* menu = lua_tostring( L, 1 );
            const char* name = lua_tostring( L, 2 );
            
            LuaFunction function( L, 3 );
            ImguiDisplay::AddImguiItem( menu, name, [function, L]( lua_State* context, std::string path, bool* control) {
                if ( L != context ) // Only execute when being called from correct callstack (luastate).
                    return;

                // Start Window
                ImguiDisplay::Call( L, [path](bool* out) {
                    return ImGui::Begin( path.c_str(), out);
                }, 0, control);

                // Call Lua
                function.Call();

                // End Window
                ImguiDisplay::Call( L, []() {
                    ImGui::End();
                    return true;
                }, 0);
            });
        }

        return 0;
    }

    int l_Text( lua_State* L )
    {
        lua_getglobal( L, "tostring" );
        lua_pushvalue( L, 2 );
        lua_call( L, 1, 1 );
        const char* str = lua_tostring( L, -1 );

        int depth = Frame( L );

        if ( str )
        {
            ImguiDisplay::Call( L, [string = std::string( str )] {
                ImGui::Text( "%s", string.c_str() );
                return true;
            }, depth );
        }
        
        return 0;
    }

    int l_Begin( lua_State* L )
    {
        int depth = PushFrame( L );
        const char* str = lua_tostring( L, 2 );
        //int depth = lua_tointeger( L, 2 );
        ImguiDisplay::Call( L, [string = std::string( str )] {
            return ImGui::Begin( string.c_str() );
            }, depth );
        return 0;
    }

    int l_End( lua_State* L )
    {
        int depth = PopFrame( L ) - 1; // Same Frame as Begin
        ImguiDisplay::Call( L, [] {
            ImGui::End();
            return true;
        }, depth );
        return 0;
    }

    template<bool (*function)( const char* )>
    int l_ControlFlowPush( lua_State* L )
    {
        int depth = PushFrame( L );
        const char* str = lua_tostring( L, 2 );
        ImguiDisplay::Call( L, [string = std::string( str )] {
            return function( string.c_str() );
            }, depth );
        return 0;
    }

    template<void ( *function )()>
    int l_ControlFlowPop( lua_State* L )
    {
        int depth = PopFrame( L );
        ImguiDisplay::Call( L, [] {
            function();
            return true;
            }, depth );
        return 0;
    }

    int l_Columns( lua_State* L )
    {
        int columns = lua_tointeger( L, 2 );
        int depth = Frame( L );
        ImguiDisplay::Call( L, [columns] {
            ImGui::Columns( columns );
            return true;
            }, depth );
        return 0;
    }

    int l_NextColumn( lua_State* L )
    {
        int depth = Frame( L );
        ImguiDisplay::Call( L, [] {
            ImGui::NextColumn();
            return true;
            }, depth );
        return 0;
    }

    int l_CreateImGui( lua_State* L )
    {
        // Create Table
        lua_newtable( L );

        static const luaL_Reg functions[] = {
            // Control
            REGISTER_CONTROL_PUSH( BeginTabBar ),
            REGISTER_CONTROL_POP( EndTabBar ),
            REGISTER_CONTROL_PUSH( BeginTabItem ),
            REGISTER_CONTROL_POP( EndTabItem ),
            REGISTER_CONTROL_PUSH( TreeNode ),
            REGISTER_CONTROL_POP( TreePop ),
            REGISTER_CONTROL_PUSH( CollapsingHeader ),
            REGISTER_FUNCTION( Pop ),

            // Special Control
            REGISTER_FUNCTION(Begin),
            REGISTER_FUNCTION(End),
            
            // Immediate
            REGISTER_FUNCTION(Text),
            REGISTER_FUNCTION(Columns),
            REGISTER_FUNCTION(NextColumn),

            // Custom
            REGISTER_FUNCTION(AddItem),
            REGISTER_FUNCTION(Refresh),
            {nullptr, nullptr}
        };

        luaL_register( L, nullptr, functions );

        lua_pushvalue( L, -1 );
        lua_setfield( L, -2, "__index" );

        lua_pushstring( L, "LuaImGui");
        lua_setfield( L, -2, "__name" );

        lua_pushinteger( L, 0 );
        lua_setfield( L, -2, "depth");
        return 1;
    }
}

