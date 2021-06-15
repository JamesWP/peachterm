cmake_minimum_required(VERSION 3.13)
project(jterm)

enable_testing()

if(UNIX)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED sdl2 SDL2_ttf fontconfig icu-uc)

set(DEPS_LIBRARIES ${DEPS_LIBRARIES} pthread boost_system)
set(platform linux)
endif(UNIX)

if(WIN32)
find_package(SDL2 REQUIRED)
find_package(SDL2_TTF REQUIRED)
find_package(GTest REQUIRED)

set(DEPS_INCLUDE_DIRS ${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})
set(DEPS_LIBRARIES ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARY} icuuc icuin)
set(platform windows)
endif(WIN32)

message(STATUS "linking: " ${DEPS_LIBRARIES})
message(STATUS "including: " ${DEPS_INCLUDE_DIRS})

# Flags ----------------
if(MSVC)
set(EXTRA_CXX_FLAGS /WX /W4 -D _CRT_SECURE_NO_WARNINGS)
else()
set(EXTRA_CXX_FLAGS -Wall -Wextra -Werror)

if(PEACHTERM_ENABLE_ASAN)
    set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer)
    set(EXTRA_LDD_FLAGS ${EXTRA_LDD_FLAGS} -fsanitize=address)
endif()
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
target_link_libraries(parser-main PRIVATE jterm gtest gmock gtest_main)
add_test(NAME parser-unit-tests COMMAND parser-main)

add_executable(util-main util.m.cpp)
target_link_libraries(util-main PRIVATE jterm gtest gmock gtest_main)
add_test(NAME util-unit-tests COMMAND util-main)

add_executable(font-main font.m.cpp)
target_link_libraries(font-main PRIVATE jterm)
