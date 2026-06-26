cmake_minimum_required(VERSION 3.16)
project(Boink VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find dependencies ─────────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)

# GLEW
find_package(GLEW REQUIRED)

# GLFW3
find_package(glfw3 3.3 REQUIRED)

# Lua 5.4 (falls back to 5.3)
find_package(Lua 5.4 QUIET)
if(NOT LUA_FOUND)
    find_package(Lua 5.3 REQUIRED)
endif()
message(STATUS "Lua found: ${LUA_INCLUDE_DIR}")

# ── Engine library (static) ───────────────────────────────────────────────────
add_library(boinklib STATIC
    src/boink_renderer.cpp
    src/boink_script.cpp
    src/boink_window.cpp
)

target_include_directories(boinklib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LUA_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
)

target_link_libraries(boinklib PUBLIC
    OpenGL::GL
    GLEW::GLEW
    glfw
    ${LUA_LIBRARIES}
)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(boink main.cpp)
target_link_libraries(boink PRIVATE boinklib)

# Copy scripts folder next to executable
add_custom_command(TARGET boink POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/scripts
        $<TARGET_FILE_DIR:boink>/scripts
    COMMENT "Copying scripts to output directory"
)

# ── Installation ──────────────────────────────────────────────────────────────
install(TARGETS boink boinklib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include/boink)
install(DIRECTORY scripts/ DESTINATION share/boink/scripts)

# ── Print usage ───────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "╔══════════════════════════════════════╗")
message(STATUS "║   Boink 3D Engine - Build configured ║")
message(STATUS "╚══════════════════════════════════════╝")
message(STATUS "  Build:   cmake --build . --config Release")
message(STATUS "  Run:     ./boink [optional_script.lua]")
message(STATUS "")
