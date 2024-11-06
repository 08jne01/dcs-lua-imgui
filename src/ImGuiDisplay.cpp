#include "ImguiDisplay.h"

#include <FmGui.hpp>
#include <functional>
#include <imgui.h>
#include "Fonts/Fonts.h"

std::optional<ImGuiDisplay> ImGuiDisplay::display = std::make_optional<ImGuiDisplay>(); // Value Initialization

void ImGuiDisplay::AddImGuiItem( const std::string& menu, const std::string& name, std::function<void()> imgui_function )
{
    if ( ! display.has_value() )
        return;

    Menu& imgui_menu = display->menus[menu];
    imgui_menu.items.emplace_back();

    MenuItem& imgui_menu_item = imgui_menu.items.back();
    imgui_menu_item.imgui_function = std::move( imgui_function );
    imgui_menu_item.name = name;

    
    Log( std::format( "[IMGUI][C++] Added {}/{}", menu, name ).c_str() );
}

void ImGuiDisplay::AddLuaImGuiItem( const std::string& menu, const std::string& name, std::function<void( lua_State*, std::string item, bool*)> imgui_function )
{
    if ( ! display.has_value() )
        return;

    LuaMenu& imgui_menu = display->lua_menus[menu];
    imgui_menu.items.emplace_back();

    LuaMenuItem& imgui_menu_item = imgui_menu.items.back();
    imgui_menu_item.imgui_function = std::move( imgui_function );
    imgui_menu_item.name = name;

    Log( std::format( "[IMGUI][Lua] Added {}/{}", menu, name ).c_str() );
}

void ImGuiDisplay::DisplayHook()
{
    if ( display )
        display->Display();
}


void ImGuiDisplay::RefreshDisplay( lua_State* L )
{
    if ( display )
        display->Refresh(L);
}

void ImGuiDisplay::SetupHook()
{
    ImGuiIO& io = ImGui::GetIO();
    void* bytes;
    size_t n_bytes = fonts::GetConsolaTTF( ImGui::MemAlloc, bytes );
    io.Fonts->AddFontFromMemoryTTF( bytes, static_cast<int>( n_bytes ), 14.0f, NULL, io.Fonts->GetGlyphRangesDefault() );
}

void ImGuiDisplay::CreateHook()
{
    if ( ! display.has_value() )
        return;

    if ( display->hook_created )
        return;

    display->hook_created = true; // creat hook only once
    FmGuiConfig config;
    config.imGuiStyle = FmGuiStyle::CLASSIC;

    if ( ! FmGui::StartupHook( config ) )
    {
        printf( "FmGui::StartupHook failed.\n" );
    }
    else
    {
        printf( "D3D11 Context: %s\n", FmGui::AddressDump().c_str() );
        FmGui::SetImGuiSetupRoutinePtr( SetupHook );
        FmGui::SetRoutinePtr( DisplayHook );
        FmGui::SetInputRoutinePtr( nullptr );
        FmGui::SetWidgetVisibility( true );
    }
}


ImGuiDisplay::~ImGuiDisplay()
{
    error = true;
    FmGui::SetInputRoutinePtr( nullptr );
    FmGui::SetRoutinePtr( nullptr );

    if ( ! FmGui::ShutdownHook() )
    {
        printf( "FmGui::ShutdownHook failed.\n" );
    }
}

void ImGuiDisplay::Refresh( lua_State* L )
{
    if ( hidden )
        return;

    if ( error )
        return;
    
    if ( menus.empty() && lua_menus.empty() )
        return;

    commands[L].clear();

    for ( auto& [menu_name, menu] : lua_menus )
    {
        for ( auto& menu_item : menu.items )
        {

            if ( ! menu_item.visible )
            {
                continue;
            }

            std::string path = menu_name + "/" + menu_item.name;
            
            //if ( ImGui::Begin( path.c_str(), &menu_item.visible ) )
            //{
            if ( menu_item.imgui_function )
                menu_item.imgui_function( L, std::move( path ), &menu_item.visible );
                //ImGui::End();
            //}
        }
    }

    // Copy to render thread.
    std::unique_lock lock( command_mtx );
    completed_commands[L] = std::move( commands[L] );

}

void ImGuiDisplay::InitializeContextFunctions()
{
    if ( ctx == nullptr )
        return;

    if ( alloc == nullptr )
        return;

    if ( plot_ctx == nullptr )
        return;

    ctx( ImGui::GetCurrentContext() );

    ImGuiMemAllocFunc p_alloc_func;
    ImGuiMemFreeFunc p_free_func;
    void* p_user_data;
    ImGui::GetAllocatorFunctions( &p_alloc_func, &p_free_func, &p_user_data );
    alloc( p_alloc_func, p_free_func, p_user_data );
    plot_ctx( ImPlot::GetCurrentContext() );
    initialize_remote_context = false;
}

void ImGuiDisplay::DrawCppImGui()
{
    for ( auto& [menu_name, menu] : menus )
    {
        for ( auto& menu_item : menu.items )
        {

            if ( ! menu_item.visible )
            {
                continue;
            }

            std::string path = menu_name + "/" + menu_item.name;

            if ( ImGui::Begin( path.c_str(), &menu_item.visible ) )
            {
                if ( menu_item.imgui_function )
                    menu_item.imgui_function();
                ImGui::End();
            }
        }
    }
}

void ImGuiDisplay::Display()
{
    if ( initialize_remote_context )
        InitializeContextFunctions();

    if ( hidden )
        return;

    if ( error )
        return;

    if ( ImGui::BeginMainMenuBar() )
    {
        if ( ImGui::BeginMenu( "File" ) )
        {
            if ( ImGui::MenuItem( "Console" ) )
            {
                console_open = true;
            }

            if ( ImGui::MenuItem( "Style Editor" ) )
            {
                style_editor_open = true;
            }

            ImGui::EndMenu();
        }

        if ( style_editor_open )
        {
            if ( ImGui::Begin( "Style Editor", &style_editor_open ) )
                ImGui::ShowStyleEditor();
            ImGui::End();
        }

       
        if ( console_open )
            console.Draw( "Console", console_open );

        auto draw_menus = [this]( auto& menus_in ) {
            for ( auto& [menu_name, menu] : menus_in )
            {
                if ( ! ImGui::BeginMenu( menu_name.c_str() ) )
                {
                    continue;
                }

                for ( auto& menu_item : menu.items )
                {
                    if ( ImGui::MenuItem( menu_item.name.c_str() ) )
                    {
                        menu_item.visible = true;
                    }
                }
                ImGui::EndMenu();
            }
        };

        draw_menus( menus );
        draw_menus( lua_menus );

        ImGui::EndMainMenuBar();
    }

    DrawCppImGui();

    std::unique_lock lock( command_mtx );
    int allowed_depth = 0;

    for ( auto& [L, L_commands] : completed_commands )
    {
        for ( auto& c : L_commands )
        {
            if ( c.depth <= allowed_depth )
            {
                if ( c.command( c.control ) )
                {
                    allowed_depth = c.depth + 1;
                }
                else
                {
                    allowed_depth = c.depth;
                }
            }
        }
    }
}