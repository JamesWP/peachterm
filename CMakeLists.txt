cmake_minimum_required(VERSION 3.13)
project(jterm)

enable_testing()

# Libraries ----------------
if(UNIX)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED sdl2 SDL2_ttf)

find_package(ICU COMPONENTS uc REQUIRED)
set(DEPS_LIBRARIES pthread boost_system)
endif(UNIX)

if(WIN32)
find_package(SDL2 REQUIRED)
find_package(SDL2_TTF REQUIRED)
find_package(GTest REQUIRED)

set(DEPS_INCLUDE_DIRS ${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})
set(DEPS_LIBRARIES ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARY} icuuc icuin)
message(STATUS "linking: " ${DEPS_LIBRARIES})
message(STATUS "including: " ${DEPS_INCLUDE_DIRS})
endif(WIN32)

# Flags ----------------
if(MSVC)
set(EXTRA_CXX_FLAGS /WX /W4 -D _CRT_SECURE_NO_WARNINGS)
else()
set(EXTRA_CXX_FLAGS -Wall -Wextra -Werror)
endif()

add_library(jterm graphics.cpp app.cpp io.cpp parser.cpp keyboard.cpp colors.cpp vterm.cpp termhistory.cpp)
target_include_directories(jterm PUBLIC ${DEPS_INCLUDE_DIRS} .)
target_link_libraries(jterm PUBLIC ${DEPS_LIBRARIES} )
target_compile_options(jterm PUBLIC ${EXTRA_CXX_FLAGS})
set_property(TARGET jterm PROPERTY CXX_STANDARD 17)

add_executable(main main.m.cpp)
target_link_libraries(main PRIVATE jterm)
target_compile_options(main PRIVATE ${EXTRA_CXX_FLAGS})
set_property(TARGET main PROPERTY CXX_STANDARD 17)

add_executable(io-main io.m.cpp)
target_link_libraries(io-main PRIVATE jterm)
target_compile_options(io-main PRIVATE ${EXTRA_CXX_FLAGS})
set_property(TARGET io-main PROPERTY CXX_STANDARD 17)

add_executable(parser-main parser.m.cpp)
target_link_libraries(parser-main PRIVATE jterm GTest::gtest GTest::gmock GTest::gtest_main)
target_compile_options(parser-main PRIVATE ${EXTRA_CXX_FLAGS})
set_property(TARGET parser-main PROPERTY CXX_STANDARD 17)
add_test(NAME parser-unit-tests COMMAND parser-main)

add_executable(util-main util.m.cpp)
target_link_libraries(util-main PRIVATE jterm GTest::gtest GTest::gtest_main)
target_compile_options(util-main PRIVATE ${EXTRA_CXX_FLAGS})
set_property(TARGET util-main PROPERTY CXX_STANDARD 17)
add_test(NAME util-unit-tests COMMAND util-main)

