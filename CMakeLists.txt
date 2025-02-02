cmake_minimum_required(VERSION 3.15)

project(kbooth LANGUAGES CXX)

set(KB_SRC_DIR "src")
set(IMGUI_SRC_DIR "lib/imgui")

if (WIN32)
	set(SDL3_INCLUDE_DIR "C:/Program Files (x86)/SDL3/include")
	set(SDL3_LIBRARY "C:/Program Files (x86)/SDL3/lib/SDL3.lib") 
	set(CMAKE_CXX_COMPILER "g++.exe")
	set(CMAKE_C_COMPILER "gcc.exe")
endif (WIN32)


if (UNIX)
	set(SDL3_INCLUDE_DIR "/usr/local/include/SDL3")
	set(SDL3_LIBRARY "/usr/local/lib/libSDL3.so") 
	set(CMAKE_CXX_COMPILER "c++")
	set(CMAKE_C_COMPILER "cc")
endif (UNIX)

set(KB_HEADERS 
	"${KB_SRC_DIR}/Camera.h"
	"${KB_SRC_DIR}/Kbooth.h"
	"${KB_SRC_DIR}/SimpleIni.h"
	"${KB_SRC_DIR}/UIWindow.h"
	"${IMGUI_SRC_DIR}/imgui.h"
	"${IMGUI_SRC_DIR}/imconfig.h"
	"${IMGUI_SRC_DIR}/imgui_internal.h"
	"${IMGUI_SRC_DIR}/imstb_textedit.h"
	"${IMGUI_SRC_DIR}/imstb_truetype.h"
	"${IMGUI_SRC_DIR}/imstb_rectpack.h"
	"${IMGUI_SRC_DIR}/imgui_impl_sdlrenderer3.h"
	"${IMGUI_SRC_DIR}/imgui_impl_sdl3.h")

set(KB_SOURCES
	"${KB_SRC_DIR}/Camera.cpp"
	"${KB_SRC_DIR}/main.cpp"
	"${KB_SRC_DIR}/UIWindow.cpp"
	"${IMGUI_SRC_DIR}/imgui.cpp"
	"${IMGUI_SRC_DIR}/imgui_demo.cpp"
	"${IMGUI_SRC_DIR}/imgui_draw.cpp"
	"${IMGUI_SRC_DIR}/imgui_tables.cpp"
	"${IMGUI_SRC_DIR}/imgui_widgets.cpp"
	"${IMGUI_SRC_DIR}/imgui_impl_sdlrenderer3.cpp"
	"${IMGUI_SRC_DIR}/imgui_impl_sdl3.cpp")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#add_definitions(-DNO_WARN_X86_INTRINSICS)

add_executable(${PROJECT_NAME} ${KB_SOURCES} ${KB_HEADERS})

target_include_directories(${PROJECT_NAME} 
	PRIVATE 
		${IMGUI_SRC_DIR}
		${SDL3_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL3_LIBRARY})
