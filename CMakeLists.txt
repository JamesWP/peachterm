cmake_minimum_required(VERSION 3.13)
project(eduterm)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED sdl2 SDL2_ttf cairo)

add_executable(main main.m.cpp)
target_include_directories(main PRIVATE ${DEPS_INCLUDE_DIRS})
target_link_libraries(main PRIVATE ${DEPS_LIBRARIES})

