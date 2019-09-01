cmake_minimum_required(VERSION 3.13)
project(jterm)

enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED sdl2 SDL2_ttf cairo)

find_package(ICU COMPONENTS uc REQUIRED)

add_library(jterm graphics.cpp app.cpp io.cpp parser.cpp keyboard.cpp colors.cpp)
target_include_directories(jterm PUBLIC ${DEPS_INCLUDE_DIRS} .)
target_link_libraries(jterm PUBLIC ${DEPS_LIBRARIES} ${ICU_LIBRARIES} pthread boost_system)
target_compile_options(jterm PUBLIC -Wall -Wextra -Werror -std=c++17)

add_executable(main main.m.cpp)
target_link_libraries(main PRIVATE jterm)
target_compile_options(main PRIVATE -Wall -Wextra -Werror)

add_executable(io-main io.m.cpp)
target_link_libraries(io-main PRIVATE jterm)
target_compile_options(io-main PRIVATE -Wall -Wextra -Werror)

add_executable(parser-main parser.m.cpp)
target_include_directories(parser-main PRIVATE /usr/include)
target_link_libraries(parser-main PRIVATE jterm gtest gmock gtest_main)
target_compile_options(parser-main PRIVATE -Wall -Wextra -Werror)
add_test(NAME parser-unit-tests COMMAND parser-main)

add_executable(util-main util.m.cpp)
target_include_directories(util-main PRIVATE /usr/include)
target_link_libraries(util-main PRIVATE jterm gtest gmock gtest_main)
target_compile_options(util-main PRIVATE -Wall -Wextra -Werror)
add_test(NAME util-unit-tests COMMAND util-main)

