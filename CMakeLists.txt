cmake_minimum_required(VERSION 3.15)

# Initialize project name dynamically from our Makefile argument
if(NOT GAME_BIN)
    set(GAME_BIN "mini_game")
endif()
project(${GAME_BIN} CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT GAME_TITLE)
    set(GAME_TITLE "Mini Game")
endif()
add_compile_definitions(WINDOW_TITLE="${GAME_TITLE}")

# ----------------------------------------------------
# 🔧 Build Profiles Configuration (Debug vs Release)
# ----------------------------------------------------

# 🗜️ RELEASE FLAGS: Maximize size minimization (-Os), drop unreferenced blocks
set(CMAKE_CXX_FLAGS_RELEASE "-Os -ffunction-sections -fdata-sections -DNDEBUG")

# 🏎️ DEBUG FLAGS: Zero optimization (-O0) for fastest builds, keep symbols (-g)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")

if(APPLE)
    # Strip symbols and dead code *only* on a Release build target
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-dead_strip")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # Strip symbols and garbage-collect unreferenced sections on Linux Release
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections -s")
endif()

# ----------------------------------------------------

include_directories(deps)
include_directories(src)

add_subdirectory(deps/minifb)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(mini_game ${SOURCES})

target_link_libraries(${GAME_BIN} PRIVATE minifb)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(X11 REQUIRED)
    target_link_libraries(${GAME_BIN} PRIVATE X11::X11 pthread GL)
endif()

# Only strip if we are building in Release mode
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET ${GAME_BIN} POST_BUILD
        COMMAND ${CMAKE_STRIP} -u -r $<TARGET_FILE:${GAME_BIN}>
        COMMENT "Stripping debug symbols from the binary..."
    )
endif()
