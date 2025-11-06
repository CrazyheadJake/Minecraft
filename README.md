# Minecraft
A simple Minecraft-style engine implemented in C/C++ with a modular structure and CMake-based build system.

## Features
- Multithreaded chunk loading
- Support for transparent, translucent, and solid blocks
- Can break/place blocks in the world
- CMake build system for cross-platform compilation
- Written in modern C/C++ for clarity and performance

## Requirements
- C/C++ compiler
- [CMake](https://cmake.org/) (3.x or later)
- Linux

## Installation
```bash
git clone https://github.com/CrazyheadJake/Minecraft.git
cd Minecraft
mkdir build && cd build
cmake ..
make
```

## Usage
```bash
./build/Minecraft
```

## License
This project is licensed under the [Apache 2.0 License](LICENSE).
