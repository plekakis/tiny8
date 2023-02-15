# What?
A header-only CHIP-8 interpreter. 

Many thanks to https://github.com/Timendus/chip8-test-suite for the test suite.

Info on CHIP-8 itself: https://en.wikipedia.org/wiki/CHIP-8

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

// getting timers:
auto const sound = interpreter.get_timers()->m_sound;
auto const delay = interpreter.get_timers()->m_delay;

// access to the registers:
auto* const registers = interpreter.get_registers();
auto const r = registers->m_v[0];

```

For a working example, see **tiny8_sample.cpp** (uses SDL for input and output).

# Screenshots
![image](https://user-images.githubusercontent.com/5764341/219083385-8dfe1977-4b22-41cf-b73c-6d92fde9400c.png)
![image](https://user-images.githubusercontent.com/5764341/219083506-8ca72553-879c-4e62-8016-39179ae2e92d.png)
![image](https://user-images.githubusercontent.com/5764341/219083583-f07cf43e-e8e2-418e-a299-386707d8529b.png)
![image](https://user-images.githubusercontent.com/5764341/219083639-3fb3df56-0be8-4ea9-af6c-db9c77c672fe.png)
![image](https://user-images.githubusercontent.com/5764341/219083733-0826c373-109c-4975-941b-6b4799dfe0ff.png)


# Bugs/Requests
Please use the [GitHub issue tracker](https://github.com/alkisbkn/tiny8/issues) to submit bugs or request features.

# License
Copyright Pantelis Lekakis, 2023

Distributed under the terms of the MIT license, yatm is free and open source software.
