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
