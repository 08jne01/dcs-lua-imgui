# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(LuaImGui DESCRIPTION "Lua Library for use with DCS to Create ImGui windows")

file(GLOB_RECURSE source_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" )
file(GLOB_RECURSE headers_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" )


set(FMGUI_SRC_PATH "${CMAKE_SOURCE_DIR}/ext/DCS-EFM-ImGui/")
set(FMGUI_SRC
    Source/FmGui.cpp
)
list(TRANSFORM FMGUI_SRC PREPEND ${FMGUI_SRC_PATH})

set(IMGUI_SRC_PATH "${CMAKE_SOURCE_DIR}/ext/imgui/")
set(IMGUI_SRC
    backends/imgui_impl_dx11.cpp
    backends/imgui_impl_win32.cpp
    imgui.cpp
    #imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)
list(TRANSFORM IMGUI_SRC PREPEND ${IMGUI_SRC_PATH})



add_library( LuaImGui SHARED
    ${source_list}
    ${IMGUI_SRC}
    ${FMGUI_SRC}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LuaImGui PROPERTY CXX_STANDARD 23)
endif()

add_subdirectory("ext/minhook")

set_target_properties( LuaImGui PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_SOURCE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}" )

target_compile_definitions(LuaImGui PRIVATE LUA_BUILD_AS_DLL)
#target_link_directories( LuaImGui PRIVATE  )
target_link_libraries( LuaImGui PRIVATE minhook "${CMAKE_SOURCE_DIR}/ext/dcs-lua/lua.lib" )
target_include_directories( LuaImGui PRIVATE
    "${CMAKE_SOURCE_DIR}/ext/imgui/"
    "${CMAKE_SOURCE_DIR}/ext/imgui/backends"
    "${CMAKE_SOURCE_DIR}/ext/DCS-EFM-ImGui/Include"
    "${CMAKE_SOURCE_DIR}/ext/dcs-lua/include"
)