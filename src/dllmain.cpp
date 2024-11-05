#include <Windows.h>

extern "C"
{
#include "lua.h"
}

#include "LuaImGui.h"
#include "ImGuiDisplay.h"
#include <optional>
#include "imgui.h"

BOOL APIENTRY DllMain( HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" int __declspec( dllexport ) luaopen_LuaImGui( lua_State* L )
{
    ImGuiDisplay::CreateHook();
    /*ImguiDisplay::AddImguiItem( "Hello", "Hello", []() {
        ImGui::Text( "Hello" );
    });*/

    return LuaImGui::l_CreateImGui( L );
}

