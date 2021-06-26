cmake_minimum_required(VERSION 3.13)
project(jterm)

enable_testing()

# Build dependencies
if(UNIX)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED sdl2 SDL2_ttf fontconfig icu-uc)

set(DEPS_LIBRARIES ${DEPS_LIBRARIES} pthread boost_system)
SET(DEPS_GTEST_LIBRARIES gtest gmock gtest_main)
set(platform linux)
endif(UNIX)

if(WIN32)
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-ttf CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

set(DEPS_INCLUDE_DIRS "")
set(DEPS_LIBRARIES SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_ttf icuuc icuin)
SET(DEPS_GTEST_LIBRARIES GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
set(platform windows)
endif(WIN32)

message(STATUS "linking: " ${DEPS_LIBRARIES})
message(STATUS "including: " ${DEPS_INCLUDE_DIRS})

# Build options
option(PEACHTERM_IS_VERBOSE "Build project with extra debugging info printed to stdout")
option(PEACHTERM_IS_VERY_VERBOSE "Build project with extra extra debugging info printed to stdout")
option(PEACHTERM_IS_SLOMO "Build project so each character is printed seperately")

# Compiler Flags 
if(MSVC)
set(EXTRA_CXX_FLAGS /WX /W4 -D _CRT_SECURE_NO_WARNINGS)
else()
set(EXTRA_CXX_FLAGS -Wall -Wextra -Werror)

if(PEACHTERM_ENABLE_ASAN)
    set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer)
    set(EXTRA_LDD_FLAGS ${EXTRA_LDD_FLAGS} -fsanitize=address)
endif()
endif()

if(PEACHTERM_IS_VERBOSE)
add_compile_definitions(PEACHTERM_IS_VERBOSE)
endif()

if(PEACHTERM_IS_VERY_VERBOSE)
add_compile_definitions(PEACHTERM_IS_VERY_VERBOSE)
endif()

if(PEACHTERM_IS_SLOMO)
add_compile_definitions(PEACHTERM_IS_SLOMO)
endif()

add_library(jterm 
    graphics.cpp 
    app.cpp 
    io_${platform}.cpp 
    fonts_${platform}.cpp
    parser.cpp 
    keyboard.cpp 
    colors.cpp 
    vterm.cpp 
    termhistory.cpp
    text_renderer.cpp)
target_include_directories(jterm PUBLIC ${DEPS_INCLUDE_DIRS} .)
target_link_options(jterm BEFORE PUBLIC ${EXTRA_LDD_FLAGS})
target_link_libraries(jterm PUBLIC ${DEPS_LIBRARIES})
target_compile_options(jterm PUBLIC ${EXTRA_CXX_FLAGS})
target_compile_features(jterm PUBLIC cxx_std_17)

add_executable(main main.m.cpp)
target_link_libraries(main PRIVATE jterm)

add_executable(io-main io.m.cpp)
target_link_libraries(io-main PRIVATE jterm)

add_executable(parser-main parser.m.cpp)
target_link_libraries(parser-main PRIVATE jterm ${DEPS_GTEST_LIBRARIES})
add_test(NAME parser-unit-tests COMMAND parser-main)

add_executable(util-main util.m.cpp)
target_link_libraries(util-main PRIVATE jterm ${DEPS_GTEST_LIBRARIES})
add_test(NAME util-unit-tests COMMAND util-main)

add_executable(font-main font.m.cpp)
target_link_libraries(font-main PRIVATE jterm)

add_executable(test-textures textures.m.cpp)
target_include_directories(test-textures PUBLIC ${DEPS_INCLUDE_DIRS} .)
target_link_options(test-textures BEFORE PUBLIC ${EXTRA_LDD_FLAGS})
target_link_libraries(test-textures PUBLIC ${DEPS_LIBRARIES})
target_compile_options(test-textures PUBLIC ${EXTRA_CXX_FLAGS})
target_compile_features(test-textures PUBLIC cxx_std_17)

add_executable(scroller exp_scroller.m.cpp)
target_include_directories(scroller PUBLIC ${DEPS_INCLUDE_DIRS} .)
target_link_options(scroller BEFORE PUBLIC ${EXTRA_LDD_FLAGS})
target_link_libraries(scroller PUBLIC ${DEPS_LIBRARIES})
target_compile_options(scroller PUBLIC ${EXTRA_CXX_FLAGS})
target_compile_features(scroller PUBLIC cxx_std_17)