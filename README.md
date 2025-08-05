# TextEditor

A simple text editor implemented in C.

## Project Structure

- `src/` - Source files for the editor
  - `main.c` - Entry point of the application
  - `editor.c` - Editor logic implementation
  - `utils.c` - Utility functions
- `include/` - Header files
  - `editor.h` - Editor function declarations
  - `utils.h` - Utility function declarations
- `Makefile` - Build instructions

## Build Instructions

To build the project, run:

```sh
make
```

This will generate the executable in the project directory.

## Usage

After building, run the editor with:

```sh
./texteditor
```
if you have a file:

```sh
./texteditor fileName.txt
```
## controls
  esc - exit
  
  ctrl -s - save file
  
  tab - 4 spaces
  
  arrows - to moving around
  
  enter - break line
  
  backspace - delete char

## Features

- Basic text editing
- Modular code structure

## Requirements

- GCC or compatible C compiler
- Make


