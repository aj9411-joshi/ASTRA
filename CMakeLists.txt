cmake_minimum_required(VERSION 3.21)
project(ASTRA VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ASTRA_BUILD_CLI "Build the console demo programs" ON)
option(ASTRA_BUILD_GUI "Build the Qt 6 GUI (needs Qt 6.3+)" ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# ---------------------------------------------------------------------------
# Core library: collision (C), graph (C), routing/Dijkstra (C++)
# ---------------------------------------------------------------------------
add_library(astra_core STATIC
    src/core/collision.c
    src/core/graph.c
    src/core/routing.cpp
)
target_include_directories(astra_core PUBLIC src/core)
if(UNIX)
    target_link_libraries(astra_core PUBLIC m)
endif()

# Make sample data available next to the built programs
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------------
# Console programs
# ---------------------------------------------------------------------------
if(ASTRA_BUILD_CLI)
    add_executable(astra_collision_cli src/cli/collision_cli.c)
    target_link_libraries(astra_collision_cli PRIVATE astra_core)

    add_executable(astra_graph_demo src/cli/graph_demo.c)
    target_link_libraries(astra_graph_demo PRIVATE astra_core)

    add_executable(astra_routing_demo src/cli/routing_demo.cpp)
    target_link_libraries(astra_routing_demo PRIVATE astra_core)
endif()

# ---------------------------------------------------------------------------
# Qt 6 GUI
# ---------------------------------------------------------------------------
if(ASTRA_BUILD_GUI)
    find_package(Qt6 6.3 QUIET COMPONENTS Widgets)
    if(Qt6_FOUND)
        qt_standard_project_setup()
        qt_add_executable(ASTRA_GUI src/gui/main.cpp)
        target_link_libraries(ASTRA_GUI PRIVATE Qt6::Widgets astra_core)
    else()
        message(WARNING "Qt 6 not found - skipping ASTRA_GUI. Console tools will still build.")
    endif()
endif()
