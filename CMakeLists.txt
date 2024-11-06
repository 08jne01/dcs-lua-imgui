# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

project(LuaImGui DESCRIPTION "Lua (and optionally C++) Library for use with DCS to Create ImGui windows")

file(GLOB_RECURSE source_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" )
file(GLOB_RECURSE headers_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" )


set(FMGUI_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext/DCS-EFM-ImGui/")
set(FMGUI_SRC
    Source/FmGui.cpp
)
list(TRANSFORM FMGUI_SRC PREPEND ${FMGUI_SRC_PATH})

set(IMPLOT_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext/implot/")
set(IMPLOT_SRC
    implot.cpp
    implot_items.cpp
)
list(TRANSFORM IMPLOT_SRC PREPEND ${IMPLOT_SRC_PATH})

set(IMGUI_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/")

set(IMGUI_SRC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)
list(TRANSFORM IMGUI_SRC PREPEND ${IMGUI_SRC_PATH})

add_library( ImGui STATIC 
    ${IMGUI_SRC} 
    ${IMPLOT_SRC}
)

target_include_directories( ImGui
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/implot"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/"
    PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/implot"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/"
)

set(IMGUI_BACKEND_SRC
    backends/imgui_impl_dx11.cpp
    backends/imgui_impl_win32.cpp
)
list(TRANSFORM IMGUI_BACKEND_SRC PREPEND ${IMGUI_SRC_PATH})

add_library( LuaImGui SHARED
    ${source_list}
    ${IMGUI_BACKEND_SRC}
    ${FMGUI_SRC}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LuaImGui PROPERTY CXX_STANDARD 23)
endif()

add_subdirectory("ext/minhook")

set_target_properties( LuaImGui PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" )

target_compile_definitions(LuaImGui PRIVATE LUA_IMGUI_EXPORT LUA_BUILD_AS_DLL FMGUI_ENABLE_IMPLOT)
#target_link_directories( LuaImGui PRIVATE  )
target_link_libraries( LuaImGui 
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/dcs-lua/lua.lib"
    minhook
    PUBLIC
    ImGui # Forward ImGui static library to caller.
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/dcs-lua/lua.lib"
)
target_include_directories( LuaImGui
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/implot"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/dcs-lua/include"
    PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/implot"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/dcs-lua/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/DCS-EFM-ImGui/Include"
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
)

MESSAGE(INFO " LUA BIN PATH: ${LUA_IMGUI_BIN_PATH}")
if ( ${LUA_IMGUI_BIN_PATH} )
    add_custom_command(TARGET LuaImGui POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        -E copy "$<TARGET_FILE:LuaImGui>" "${LUA_IMGUI_BIN_PATH}/$<TARGET_FILE_NAME:LuaImGui>"
        COMMENT "Copying LuaImGui to bin"
    )
endif()