# PeachTerm

🍑 Term - a terminal emulator written in c++ - bruises easily.

# build requirements

- `cmake (3.13)`
- `c++ compiler (-std=c++17)`
- `libsdl2`
- `libsdl2_ttf`
- `libboost`
- `libicu`
- `libgtest`
- `libgmock`

# build instructions
```sh
$ mkdir bld
$ cd bld
$ cmake ..
$ cmake --build .
$ ./main

```

# todo lists
- scrollback
- display termsize on change
- add fading toasts for info messages
- add record mode for debug and replay
- redraw screen when exposed again.

## CSI codes
```
'@'
'B'
'A'
'P'
'm'
'J'
'c'
'C'
'H'
'K'
'r'
'l'
's'
'h'
'M'
'L'
'n'
't'
```

## Building on windows

Cmake modules:
- FindSDL2.cmake
- FindSDL2TTF.cmake

Configure args:
- -DCMAKE_LIBRARY_ARCHITECTURE=x86
- -DCMAKE_MODULE_PATH=C:\SomePath\cmake-modules\

Env vars:
- SDL2DIR=C:\SomePath\SDL2-2.0.14
- SDL2TTFDIR=C:\SomePath\SDL2_ttf-2.0.15
- GTEST_ROOT=C:\SomePath\googletest-distribution


_n.b. when building gtest from source is it sometimes required to pass '-Dgtest_force_shared_crt=yes' to cmake whn configuring the build_