# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.22)
project(fritchii_cinematic_model_probe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.8.3 REQUIRED COMPONENTS Core Gui Test)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(CINEMATIC_SRC_DIR "${REPO_ROOT}/src/yuzu/cinematic")

add_executable(cinematic_model_probe
    model_probe.cpp
    "${CINEMATIC_SRC_DIR}/cinematic_game_model.h"
    "${CINEMATIC_SRC_DIR}/cinematic_game_model.cpp"
)

target_include_directories(cinematic_model_probe PRIVATE
    "${CINEMATIC_SRC_DIR}"
)

target_link_libraries(cinematic_model_probe PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Test
)

enable_testing()
add_test(NAME cinematic_model_probe COMMAND cinematic_model_probe)
