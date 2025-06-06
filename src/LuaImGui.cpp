#include "LuaImGui.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

#include <format>
#include "imgui.h"
#include "implot.h"

#include "ImGuiDisplay.h"

//#define REGISTER_FUNCTION( L, function ) \
//    lua_pushcfunction(L, l_##function ); \
//    lua_setfield( L, -2, #function )
//
//#define REGISTER_FUNCTION_NAMED(L, function, name) \
//    lua_pushcfunction(L, function) \
//    lua_setfield(L, -2, name)

#define REGISTER_FUNCTION(function) { #function, l_ ## function }

#define CONTROL_PUSH( function ) l_ControlFlowPush<[]( const char* c ) { return ImGui::function(c); }>
#define CONTROL_POP( function ) l_ControlFlowPop<[]() { return ImGui::function(); }>

#define REGISTER_CONTROL_PUSH(function) { #function, CONTROL_PUSH(function)}
#define REGISTER_CONTROL_POP(function) { #function, CONTROL_POP(function)}

namespace LuaImGui
{

    template<typename T>
    T ToValue( lua_State* L, int idx );

    template<>
    double ToValue( lua_State* L, int idx )
    {
        return lua_tonumber(L, idx);
    }

    template<>
    const char* ToValue( lua_State* L, int idx )
    {
        return lua_tostring(L, idx);
    }

    template<>
    int ToValue( lua_State* L, int idx )
    {
        return lua_tointeger(L, idx);
    }

    template<typename T>
    std::vector<T> ReadVector( lua_State* L, int idx )
    {
        std::vector<T> values;
        for ( int i = 1; ; i++ )
        {
            lua_rawgeti( L, idx, i );
            if ( lua_isnil( L, -1 ) )
            {
                lua_pop( L, 1 );
                break;
            }

            const T y = ToValue<T>( L, -1 );
            values.emplace_back( y );
            lua_pop( L, 1 );
        }

        return values;
    }

    void LogError( lua_State* L, int idx )
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

    // Longjumps away never returns
    void RaiseError( lua_State* L, const char* str )
    {
        lua_Debug debug_info;
        lua_getstack(L, 1, &debug_info);
        lua_getinfo(L, "l", &debug_info);
        lua_pushstring( L, std::format("Line {} -> {}", debug_info.currentline, str ).c_str() );
        lua_error(L);
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
                LogError( L, lua_gettop(L) );
                lua_pop( L, 1 );
                ImGuiDisplay::Error();
            }
        }
    };

    int Frame( lua_State* L )
    {
        lua_getfield( L, 1, "depth" );
        //Print( L, -1 );
        int depth = int(lua_tointeger( L, -1 ));
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
        ImGuiDisplay::RefreshDisplay( L );
        return 0;
    }

    int l_AddItem( lua_State* L )
    {
        if ( ! lua_isstring( L, 1 ) )
        {
            RaiseError( L, "String Expected for Param 1: Did you call ImGui:AddItem? Should be ImGui.AddItem" );
            return 0;
        }

        if ( ! lua_isstring( L, 2 ) )
        {
            RaiseError( L, "String Expected for Param 2" );
            return 0;
        }

        if ( ! lua_isfunction( L, 3 ) )
        {
            RaiseError( L, "Function Expected for Param 3: Did you call ImGui:AddItem? Should be ImGui.AddItem" );
            return 0;
        }

        const char* menu = lua_tostring( L, 1 );
        const char* name = lua_tostring( L, 2 );
        float width = luaL_optnumber( L, 4, 500 );
        float height = luaL_optnumber( L, 5, 300 );

        LuaFunction function( L, 3 );
        ImGuiDisplay::AddLuaImGuiItem( menu, name, [function, L, width, height]( lua_State* context, std::string path, bool* control ) {
            if ( L != context ) // Only execute when being called from correct callstack (luastate).
                return;

            // Start Window
            ImGuiDisplay::Call( L, [path, width, height]( bool* out ) {
                ImGui::SetNextWindowSize( ImVec2(width, height), ImGuiCond_FirstUseEver );
                return ImGui::Begin( path.c_str(), out );
                }, 0, control );

            // Call Lua
            function.Call();

            // End Window
            ImGuiDisplay::Call( L, []() {
                ImGui::End();
                return true;
            }, 0 );
        } );

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
            ImGuiDisplay::Call( L, [string = std::string( str )] {
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
        ImGuiDisplay::Call( L, [string = std::string( str )] {
            return ImGui::Begin( string.c_str() );
            }, depth );
        return 0;
    }

    int l_End( lua_State* L )
    {
        int depth = PopFrame( L ) - 1; // Same Frame as Begin
        ImGuiDisplay::Call( L, [] {
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
        ImGuiDisplay::Call( L, [string = std::string( str )](bool*) {
            return function( string.c_str() );
            }, depth );
        return 0;
    }

    template<void ( *function )()>
    int l_ControlFlowPop( lua_State* L )
    {
        int depth = PopFrame( L );
        ImGuiDisplay::Call( L, [] {
            function();
            return true;
            }, depth );
        return 0;
    }

    int l_Columns( lua_State* L )
    {
        int columns = int(lua_tointeger( L, 2 ));
        int depth = Frame( L );
        ImGuiDisplay::Call( L, [columns] {
            ImGui::Columns( columns );
            return true;
            }, depth );
        return 0;
    }

    int l_NextColumn( lua_State* L )
    {
        int depth = Frame( L );
        ImGuiDisplay::Call( L, [] {
            ImGui::NextColumn();
            return true;
            }, depth );
        return 0;
    }

    int l_MenuBar( lua_State* L )
    {
        if ( ! lua_isboolean( L, 1 ) )
        {
            RaiseError( L, "Bool Expected for Param 1: Did you call with ImGui:MenuBar? Should be ImGui.MenuBar" );
            lua_pop( L, 1 );
            return 0;
        }

        bool state = lua_toboolean( L, 1 );
        ImGuiDisplay::MenuBar( state );
        return 0;
    }

    

    int l_BeginPlot( lua_State* L )
    {
        int depth = PushFrame( L );

        const char* plot_name_str = lua_tostring( L, 2 );
        const char* x_axis_str = lua_tostring( L, 3 );
        const char* y_axis_str = lua_tostring( L, 4 );
        float width = float(lua_tonumber( L, 5 ));

        ImGuiDisplay::Call( L, [
            plot_name_string = std::string( plot_name_str ),
            x_axis_string = std::string( x_axis_str ),
            y_axis_string = std::string( y_axis_str ),
            width
        ] {
            if ( ImPlot::BeginPlot( plot_name_string.c_str(), ImVec2(width, 0), ImPlotFlags_NoBoxSelect | ImPlotFlags_AntiAliased ) )
            {
                ImPlot::SetupAxis(ImAxis_X1, x_axis_string.c_str());
                ImPlot::SetupAxis(ImAxis_Y1, y_axis_string.c_str());
                return true;
            }
            return false;
        }, depth );

        return 0;
    }

    int l_EndPlot( lua_State* L )
    {
        int depth = PopFrame( L );
        const char* str = lua_tostring( L, 2 );
        ImGuiDisplay::Call( L, [] {
                ImPlot::EndPlot();
                return true;
            }, depth );

        return 0;
    }

    int l_ListBoxInternal(lua_State* L)
    {
        const int depth = Frame( L );
        const char* source_identifier = lua_tostring(L, 2);
        const char* name_str = lua_tostring( L, 3 );
        const int value = int(lua_tointeger(L, 4));

        
        auto* result = ImGuiDisplay::GetResult(L, source_identifier);
        if ( result == nullptr )
            return 0;

        if ( ! std::holds_alternative<int>(result->data) )
            result->data = value;

        std::vector<const char*> v = ReadVector<const char*>(L, 5);
        std::vector<std::string> strings_vec(v.begin(), v.end());

        ImGuiDisplay::Call(L, [=, name = std::string(name_str), strings = std::move(strings_vec)]{
            int value_mut = value-1;
            std::vector<const char*> string_ptrs;
            string_ptrs.reserve(strings.size());
            for ( auto& string : strings )
                string_ptrs.push_back(string.c_str());

            const bool changed = ImGui::ListBox(
                name.c_str(), 
                &value_mut,
                string_ptrs.data(),
                static_cast<int>(string_ptrs.size())
            );
            if ( changed ) // set only if changed
                result->data = value_mut+1;
        }, depth);

        lua_pushinteger(L, std::get<int>(result->data));
        return 1;
    }

    int l_DragFloatInternal(lua_State* L)
    {
        const int depth = Frame( L );
        const char* source_identifier = lua_tostring(L, 2);
        const char* name_str = lua_tostring( L, 3 );
        const double value = lua_tonumber(L, 4);
        const double speed = luaL_optnumber(L, 5, 1.0);
        const double min = luaL_optnumber(L, 6, 0.0);
        const double max = luaL_optnumber(L, 7, 0.0);

        auto* result = ImGuiDisplay::GetResult(L, source_identifier);
        if ( result == nullptr )
            return 0;

        if ( ! std::holds_alternative<double>(result->data) )
            result->data = value;

        ImGuiDisplay::Call(L, [=, name = std::string(name_str)]{
            auto valuef = static_cast<float>(value);
            const bool changed = ImGui::DragFloat(
                name.c_str(), 
                &valuef, 
                static_cast<float>(speed),
                static_cast<float>(min),
                static_cast<float>(max)
            );
            if ( changed ) // set only if changed
                result->data = static_cast<double>(valuef);
        }, depth);

        lua_pushnumber(L, std::get<double>(result->data));
        return 1;
    }

    int l_InputFloatInternal(lua_State* L)
    {
        const int depth = Frame( L );
        const char* source_identifier = lua_tostring(L, 2);
        const char* name_str = lua_tostring( L, 3 );
        const double value = lua_tonumber(L, 4);
        const double step = luaL_optnumber(L, 5, 0.0);
        const double step_fast = luaL_optnumber(L, 6, 0.0);

        auto* result = ImGuiDisplay::GetResult(L, source_identifier);
        if ( result == nullptr )
            return 0;

        if ( ! std::holds_alternative<double>(result->data) )
            result->data = value;

        ImGuiDisplay::Call(L, [=, name = std::string(name_str)]{
            double value_mut = value;
            const bool changed = ImGui::InputDouble(
                name.c_str(), 
                &value_mut, 
                step,
                step_fast
            );
            if ( changed ) // set only if changed
                result->data = value_mut;
        }, depth);

        lua_pushnumber(L, std::get<double>(result->data));
        return 1;
    }


    int l_ButtonInternal(lua_State* L)
    {
        const int depth = Frame( L );
        const char* source_identifier = lua_tostring(L, 2);
        const char* button_name_str = lua_tostring( L, 3 );
        
        auto* result = ImGuiDisplay::GetResult(L, source_identifier);

        if ( result == nullptr )
            return 0;
        
        ImGuiDisplay::Call(L, [result, button_name = std::string(button_name_str)]{

            if ( std::holds_alternative<bool>( result->data ) )
                std::get<bool>(result->data) |= ImGui::Button(button_name.c_str());
            else
                result->data = ImGui::Button(button_name.c_str());
        }, depth);

        lua_pushboolean(L, std::get<bool>(result->data));
        std::get<bool>(result->data) = false; // consume 
        return 1;
    }

    int l_PlotLine( lua_State* L )
    {
        const int depth = Frame( L );

        const char* line_name_str = lua_tostring( L, 2 );
        const double dx = lua_tonumber( L, 3 );

        ImGuiDisplay::Call( L, [
            line_name_string = std::string( line_name_str ),
            dx,
            y_values = std::move( ReadVector<double>( L, 4 ) )
        ] {
                ImPlot::PlotLine( line_name_string.c_str(), y_values.data(), static_cast<int>( y_values.size() ), dx );
                return true;
            }, depth );

        return 0;
    }

    int l_PlotVLines( lua_State* L )
    {
        const int depth = Frame( L );
        const char* line_name_str = lua_tostring( L, 2 );
        ImGuiDisplay::Call( L, [
            line_name_string = std::string( line_name_str ),
            positions = std::move(ReadVector<double>( L, 3 ))
        ] {
            ImPlot::PlotVLines( line_name_string.c_str(), positions.data(), static_cast<int>( positions.size() ) );
            return true;
        }, depth );

        return 0;
    }

    int l_PlotHLines( lua_State* L )
    {
        const int depth = Frame( L );
        const char* line_name_str = lua_tostring( L, 2 );
        ImGuiDisplay::Call( L, [
            line_name_string = std::string( line_name_str ),
            positions = std::move( ReadVector<double>( L, 3 ) )
        ] {
                ImPlot::PlotHLines( line_name_string.c_str(), positions.data(), static_cast<int>( positions.size() ) );
                return true;
        }, depth );

        return 0;
    }

    int l_Log( lua_State* L )
    {
        lua_getglobal( L, "tostring" );
        lua_pushvalue( L, 1 );
        lua_call( L, 1, 1 );

        const char* str = lua_tostring( L, -1 );
        ImGuiDisplay::Log( str );
        lua_pop( L, 1 );
        return 0;
    }

    int l_CreateImGui( lua_State* L )
    {
        // Create Table
        lua_newtable( L );

        static const luaL_Reg functions[] = {
            // Control
            REGISTER_CONTROL_PUSH(BeginTabBar),
            REGISTER_CONTROL_POP(EndTabBar),
            REGISTER_CONTROL_PUSH(BeginTabItem),
            REGISTER_CONTROL_POP(EndTabItem),
            REGISTER_CONTROL_PUSH(TreeNode),
            REGISTER_CONTROL_POP(TreePop),
            REGISTER_CONTROL_PUSH(CollapsingHeader),
            REGISTER_FUNCTION( Pop ),

            REGISTER_FUNCTION( BeginPlot ),
            REGISTER_FUNCTION( EndPlot ),
            
            
            REGISTER_FUNCTION( PlotLine ),
            REGISTER_FUNCTION( PlotHLines ),
            REGISTER_FUNCTION( PlotVLines ),
            
            // Special Control
            REGISTER_FUNCTION(Begin),
            REGISTER_FUNCTION(End),
            
            // Immediate
            REGISTER_FUNCTION( ButtonInternal ),
            REGISTER_FUNCTION( DragFloatInternal ),
            REGISTER_FUNCTION( ListBoxInternal ),
            REGISTER_FUNCTION( InputFloatInternal ),
            REGISTER_FUNCTION(Text),
            REGISTER_FUNCTION(Columns),
            REGISTER_FUNCTION(NextColumn),

            // Custom
            REGISTER_FUNCTION(AddItem),
            REGISTER_FUNCTION(Refresh),
            REGISTER_FUNCTION(MenuBar),
            REGISTER_FUNCTION(Log),
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

