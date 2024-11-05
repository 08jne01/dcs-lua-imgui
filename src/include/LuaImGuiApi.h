#pragma once
#ifdef LUA_IMGUI_EXPORT
#define LUA_IMGUI_API __declspec(dllexport)
#else
#define LUA_IMGUI_API __declspec(dllimport)
#endif