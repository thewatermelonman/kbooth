cmake_minimum_required(VERSION 3.15)

project(kbooth LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(KB_SRC "src")
set(KB_EXTERNAL "external")

set(KB_HEADERS 
	"${KB_SRC}/Camera.h"
	"${KB_SRC}/Kbooth.h"
	"${KB_SRC}/SimpleIni.h"
	"${KB_SRC}/UIWindow.h"
	"${KB_EXTERNAL}/imgui/imgui.h"
	"${KB_EXTERNAL}/imgui/imconfig.h"
	"${KB_EXTERNAL}/imgui/imgui_internal.h"
	"${KB_EXTERNAL}/imgui/imstb_textedit.h"
	"${KB_EXTERNAL}/imgui/imstb_truetype.h"
	"${KB_EXTERNAL}/imgui/imstb_rectpack.h"
	"${KB_EXTERNAL}/imgui/imgui_impl_sdlrenderer3.h"
	"${KB_EXTERNAL}/imgui/imgui_impl_sdl3.h")

set(KB_SOURCES
	"${KB_SRC}/Camera.cpp"
	"${KB_SRC}/main.cpp"
	"${KB_SRC}/UIWindow.cpp"
	"${KB_EXTERNAL}/imgui/imgui.cpp"
	"${KB_EXTERNAL}/imgui/imgui_demo.cpp"
	"${KB_EXTERNAL}/imgui/imgui_draw.cpp"
	"${KB_EXTERNAL}/imgui/imgui_tables.cpp"
	"${KB_EXTERNAL}/imgui/imgui_widgets.cpp"
	"${KB_EXTERNAL}/imgui/imgui_impl_sdlrenderer3.cpp"
	"${KB_EXTERNAL}/imgui/imgui_impl_sdl3.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>") 

add_subdirectory(${KB_EXTERNAL}/SDL EXCLUDE_FROM_ALL)
add_subdirectory(${KB_EXTERNAL}/SDL_image EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME} ${KB_SOURCES} ${KB_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${KB_EXTERNAL}/imgui)
#target_include_directories(${PROJECT_NAME} PRIVATE ${KB_EXTERNAL}/SDL/include)
#target_include_directories(${PROJECT_NAME} PRIVATE ${KB_EXTERNAL}/SDL_image/include)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_image::SDL3_image SDL3::SDL3)
