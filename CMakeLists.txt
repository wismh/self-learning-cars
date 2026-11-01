cmake_minimum_required(VERSION 3.20)
project(self-learning-cars LANGUAGES CXX)

# The Wind SDK. The editor passes CMAKE_PREFIX_PATH when it builds the game; CMakeUserPresets.json does for an IDE.
find_package(Wind REQUIRED)

engine_add_game(self-learning-cars
    src/main.cpp
    src/game.h
    src/game.cpp
    src/loc/tr.h
    src/loc/locale_prefs.h
    src/loc/locale_prefs.cpp
    src/ai/neural_network.h
    src/ai/genetic_algorithm.h
    src/ai/genetic_algorithm.cpp
    src/track/geometry.h
    src/track/track_generator.h
    src/track/track_generator.cpp
    src/ecs/components.h
    src/ecs/spawner.h
    src/ecs/spawner.cpp
    src/ecs/car_sensors_system.h
    src/ecs/car_sensors_system.cpp
    src/ecs/car_brain_system.h
    src/ecs/car_brain_system.cpp
    src/ecs/car_physics_system.h
    src/ecs/car_physics_system.cpp
    src/ecs/ray_render_system.h
    src/ecs/ray_render_system.cpp
    src/ecs/evolution_system.h
    src/ecs/evolution_system.cpp
    src/ecs/simulation_systems.h
    src/ecs/simulation_systems.cpp
    src/view/car_view_model.h
    src/view/car_view_model.cpp
    src/help/help_topic_table.h
    src/help/help_view_model.h
    src/help/help.h
    src/help/help.cpp
    src/panels/simulation_panel.h
    src/panels/simulation_panel.cpp)

target_include_directories(self-learning-cars PRIVATE src)

# Unit tests of the simulation. Off by default so that the editor build does not compile them.
option(CARS_TESTS "Build the unit tests" OFF)
if(CARS_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

