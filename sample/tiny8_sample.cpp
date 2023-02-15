// MIT License
// 
// Copyright(c) 2023, Pantelis Lekakis
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include <tiny8.h>
#include <SDL.h>

#include <fstream>

using namespace std;

constexpr size_t c_windowScale = 10;
constexpr size_t c_windowWidth = tiny8::c_displayWidth * c_windowScale;
constexpr size_t c_windowHeight = tiny8::c_displayHeight * c_windowScale;

// Set an sdl surface's pixel based on a given state (on/off)
void set_sdl_pixel(SDL_Surface* surface, uint32_t x, uint32_t y, uint8_t pixelState)
{
	auto* const pixel = (uint32_t*)((uint8_t*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
	*pixel = pixelState ? ~0u : 0;
}

int main(int argc, char** argv)
{
	tiny8::interpreter interpreter (tiny8::interpreter::chip8_xochip);

	ifstream rom("roms/chip8-test-suite.ch8", fstream::in | fstream::binary);
	if (rom.is_open())
	{		
		rom.seekg(0, ios::end);
		size_t const romDataSize = rom.tellg();
		rom.seekg(0, ios::beg);		

		char* const romData = reinterpret_cast<char* const>(interpreter.get_memory()->m_rom);
		rom.read(romData, romDataSize);
		rom.close();
	}

	// Initialise SDL and get hold of the window's surface.
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
	SDL_Window* window = SDL_CreateWindow("Tiny8 Sample", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, c_windowWidth, c_windowHeight, SDL_WINDOW_SHOWN);
	SDL_Surface* surface = SDL_GetWindowSurface(window);

	// This will be filled with data from the tiny8 display and will be stretched on the window surface.
	SDL_Surface* tiny8_surface = SDL_CreateRGBSurface(0, tiny8::c_displayWidth, tiny8::c_displayHeight, 32, 0, 0, 0, 0);
	
	// Setup keys to send to the interpreter. These are arranged in the following way (schematic below based on qwerty layout):
		/*
		* 1 2 3 4          1 2 3 C
		* Q W E R     ->   4 5 6 D
		* A S D F          7 8 9 E 
		* Z X C V          A 0 B F
		*
		*/

	SDL_Scancode const key_scancodes[] =
	{
		SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
		SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
		SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
		SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V,
	};
	uint8_t const key_remap[] =
	{
		1, 2, 3, 12,
		4, 5, 6, 13,
		7, 8, 9, 14,
		10, 0, 11, 15
	};
	uint8_t key_states[tiny8::c_maxKeys] = { 0 };
	
	SDL_Event e; 
	bool quit = false;
	while (quit == false) 
	{ 
		while (SDL_PollEvent(&e)) 
		{ 
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				uint8_t index = 0;
				for (auto& scancode : key_scancodes)
				{
					if (scancode == SDL_GetScancodeFromKey(e.key.keysym.sym))
					{
						key_states[key_remap[index]] = e.key.state == SDL_PRESSED ? 1 : 0;
					}
					++index;
				}
				break;
			}
		} 
	
		interpreter.advance(key_states);
	
		// Update the tiny8 surface data.
		for (uint32_t x = 0; x < tiny8::c_displayWidth; ++x)
		{
			for (uint32_t y = 0; y < tiny8::c_displayHeight; ++y)
			{
				uint8_t const value = interpreter.get_display()->m_data[y * tiny8::c_displayWidth + x];
				set_sdl_pixel(tiny8_surface, x, y, value);
			}
		}
		
		// Stretch to output.
		SDL_Rect dest;
		dest.x = 0;
		dest.y = 0;
		dest.w = c_windowWidth;
		dest.h = c_windowHeight;
		SDL_BlitScaled(tiny8_surface, NULL, surface, &dest);

		SDL_UpdateWindowSurface(window);
	}

	SDL_FreeSurface(tiny8_surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
