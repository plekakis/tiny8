# What?
A header-only CHIP-8 interpreter.

# Why?
For fun mostly, I really needed a break from another project of mine.

# Features
This is a CHIP-8, S-CHIP, XO-CHIP compatible interpreter implementing the original instruction set.
Differences across the CHIP-8 versions are also handled correctly (to the best of my knowledge).
What's missing (but planned) is full extended instruction set and also support for the higher 128x64 resolution of the later versions.

# Usage

```cpp
tiny8::interpreter interpreter(flags::tiny8::interpreter::chip8_original);

// load rom from file
...
// copy to destination
memcpy(interpreter.get_memory()->m_rom, rom, romSizeInBytes);

// advance the interpreter
interpreter.advance(keys);

// draw pixels using your favourite library
uint8_t const value = interpreter.get_display()->m_data[y * tiny8::c_displayWidth + x];

```

For a working example, see **tiny8_sample.cpp** (uses SDL for input and output).

# Bugs/Requests
Please use the [GitHub issue tracker](https://github.com/alkisbkn/tiny8/issues) to submit bugs or request features.

# License
Copyright Pantelis Lekakis, 2023

Distributed under the terms of the MIT license, yatm is free and open source software.
