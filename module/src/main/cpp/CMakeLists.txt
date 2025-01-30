cmake_minimum_required(VERSION 3.31.0)

if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not set")
else ()
    project(${MODULE_NAME})
endif ()

message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 20)

set(LINKER_FLAGS "-Wl")
set(C_FLAGS "-Werror=none -fdata-sections -ffunction-sections")
set(CXX_FLAGS "${CXX_FLAGS} -fno-exceptions")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(C_FLAGS "${C_FLAGS} -O2 -fvisibility=hidden -fvisibility-inlines-hidden")
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
else ()
    set(C_FLAGS "${C_FLAGS}")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS} ${CXX_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")

include_directories(
        Dobby/include
        imgui
        imgui/backends
        xdl/include
        KittyMemory
)

aux_source_directory(xdl xdl-src)

option(DOBBY_GENERATE_SHARED "Build shared library" ON)
option(DOBBY_DEBUG "Enable debug logging" OFF)
#option(NearBranch "Enable near branch trampoline" ON)
#option(FullFloatingPointRegisterPack "Save and pack all floating-point registers" OFF)
#option(Plugin.SymbolResolver "Enable symbol resolver" ON)
#option(Plugin.ImportTableReplace "Enable import table replace " OFF)
#option(Plugin.Android.BionicLinkerUtil "Enable android bionic linker util" OFF)
#option(BUILD_EXAMPLE "Build example" OFF)
#option(BUILD_TEST "Build test" OFF)

set(IMGUI_SRC
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_android.cpp
        imgui/backends/imgui_impl_opengl3.cpp
)

set(MENU_SRC
        plutonium/gui/menu/ui.cpp
        plutonium/gui/menu/util.cpp
        plutonium/gui/menu/widgets.cpp
)

file(GLOB KITTY_MEMORY_SRC ${PROJECT_SOURCE_DIR}/KittyMemory/*.cpp)

add_library(${MODULE_NAME} SHARED main.cpp plutonium/plutonium.cpp utils.cpp ${MENU_SRC} ${IMGUI_SRC} ${xdl-src} ${KITTY_MEMORY_SRC})

macro(SET_OPTION option value)
    set(${option} ${value} CACHE INTERNAL "" FORCE)
endmacro()

include_directories(
        .
)

find_library(log-lib log)
find_library(egl EGL)
find_library(android android)
find_library(GLESV2_LIB GLESv2)
find_library(LZ_LIB z)
if (${ANDROID_PLATFORM_LEVEL} LESS 12)
    message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 /
                      (currently using ${ANDROID_PLATFORM_LEVEL}).")
    return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
    add_definitions("-DDYNAMIC_ES3")
    set(GL3STUB_SRC gl3stub.c)
    set(OPENGL_LIB GLESv2)
else ()
    set(OPENGL_LIB GLESv3)
endif (${ANDROID_PLATFORM_LEVEL} LESS 12)

file(GLOB DOBBY_SRC ${PROJECT_SOURCE_DIR}/dobby/arm64-v8a/libdobby.a)

target_link_libraries(${MODULE_NAME} ${DOBBY_SRC} log ${LZ_LIB}
        ${OPENGL_LIB}
        ${GLESV2_LIB}
        ${android}
        ${egl})

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all --remove-section=.comment "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${MODULE_NAME}.so")
endif ()
