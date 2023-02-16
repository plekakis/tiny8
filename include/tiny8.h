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
#pragma once

#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <functional>
#include <cassert>
#include <chrono>

/*
* A fully featured CHIP-8 interpreter covering instructions for:
* - Chip8
* - SChip
* - XoChip
* 
* At the time of writing, the only thing that differentiates across versions is the instruction compatibility. Extended feature set is not yet implemented, but planned.
*/
namespace tiny8
{
	static void unimplemented(uint16_t opcode)
	{
		printf("Unimplemented instruction! Opcode: 0x%04x\n", opcode);
		assert(false);
	}

	// Memory constants
	constexpr size_t	c_maxMemory = 0x1000;
	constexpr size_t	c_maxStack = 0x400;
	constexpr uint16_t	c_romStartAddress = 0x200;

	// Font constants & data.
	constexpr size_t	c_fontStartAddress = 0x0;
	constexpr uint8_t	c_fontset[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F 
	};

	// Display constants
	constexpr size_t	c_displayWidth = 64;
	constexpr size_t	c_displayHeight = 32;
	constexpr size_t	c_displaySize = c_displayWidth * c_displayHeight;

	// Input constants
	constexpr size_t	c_maxKeys = 16;

	// Anything memory related (including font data and stack).
	struct memory
	{
		uint8_t			m_data[c_maxMemory];	// Main working memory of the CHIP-8.
		uint16_t		m_stack[c_maxStack];	// Stack is intentionally placed outside working memory; I don't know of any programs that depend on it being part of the main memory.

		uint8_t* const  m_rom = &m_data[c_romStartAddress];		// Where the rom data starts.
		uint8_t* const  m_font = &m_data[c_fontStartAddress];	// Where the font data starts.
	};

	// Display framebuffer.
	struct display
	{
		uint8_t m_data[c_displaySize];
	};

	// Registers.
	struct registers
	{
		uint16_t		m_index = 0;
		uint16_t		m_sp = 0;
		uint16_t		m_pc = c_romStartAddress;

		uint8_t			m_v[16];
	};

	// Timers.
	struct timers
	{
		uint8_t m_delay = 0;
		uint8_t m_sound = 0;
	};

	// Input keys state.
	struct input
	{
		uint8_t m_key[c_maxKeys];
		uint8_t m_prev_key[c_maxKeys];
	};

	// Represents the current decoding state.
	struct decode_state
	{
		uint16_t m_opcode;
		uint8_t  m_x;		// Used for register lookup (second nibble).
		uint8_t  m_y;		// Used for register lookup (third nibble).
		uint8_t  m_n;		// Used (fourth nibble)
		uint8_t  m_nn;		// Used to identify instructions within a family (third and fourth nibble).
		uint16_t  m_nnn;	// Used for memory access, 12 bit (second, third and fourth nibbles).
	};

	// Represents an instruction, along with a callback for execution. 
	struct instruction
	{
		std::function<void()>		m_body;
		struct instruction_family*	m_family;
	};

	// Represents an instruction family, denoted by the first nibble of the opcode. A family can have one or more instructions based on certain opcode nibbles.
	struct instruction_family
	{
		std::unordered_map<uint8_t, instruction> m_instructions;
		uint16_t	m_opcodeMask;
	};

	// The chip-8 interpreter.
	class interpreter
	{
	public:
		// Instruction compatibility and interpreter mode flags
		// All the "legacy" ones refer to the original CHIP-8 implementation
		// Relevant read on how those are assembled for each mode: https://games.gulrak.net/cadmium/chip8-opcode-table.html
		enum flags : uint8_t
		{
			none = 0,
			shift_legacy = 1 << 0,
			store_load_legacy = 1 << 1,
			jump_offset_legacy = 1 << 2,
			logical_legacy = 1 << 3,
			disp_sync_legacy = 1 << 4,
			draw_legacy = 1 << 5,
			all_legacy = shift_legacy | store_load_legacy | jump_offset_legacy | logical_legacy | disp_sync_legacy | draw_legacy,
			// operation modes
			chip8_original = all_legacy,
			chip8_schip = draw_legacy,
			chip8_xochip = store_load_legacy | jump_offset_legacy | shift_legacy
		};

		// Constructor - initialise the chip-8 interpreter internal data.
		interpreter(flags behaviour_flags = flags::none) : m_flags(behaviour_flags)
		{
			memset(m_memory.m_data, 0, sizeof(m_memory.m_data));
			memset(m_memory.m_stack, 0, sizeof(m_memory.m_stack));
			memcpy(m_memory.m_font, c_fontset, sizeof(c_fontset));

			memset(m_display.m_data, 0, sizeof(m_display.m_data));

			memset(m_registers.m_v, 0, sizeof(m_registers.m_v));

			memset(m_input.m_key, 0, sizeof(m_input.m_key));
			memset(m_input.m_prev_key, 0, sizeof(m_input.m_prev_key));

			// setup instruction families and callbacks
			// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
			add_instruction(0x00, 0xe0, 0x00ff, [&]() { memset(m_display.m_data, 0, sizeof(m_display.m_data)); });
			add_instruction(0x00, 0xee, 0x00ff, [&]() {	m_registers.m_pc = m_memory.m_stack[--m_registers.m_sp]; });
			add_instruction(0x00, 0x00, 0x0000, [&]() { unimplemented(m_state.m_opcode); });
			add_instruction(0x10, 0x00, 0x0000, [&]() { m_registers.m_pc = m_state.m_nnn; });
			add_instruction(0x20, 0x00, 0x0000, [&]() { m_memory.m_stack[m_registers.m_sp++] = m_registers.m_pc; m_registers.m_pc = m_state.m_nnn; });
			add_instruction(0x30, 0x00, 0x0000, [&]() { if (m_registers.m_v[m_state.m_x] == m_state.m_nn) m_registers.m_pc += 2; });
			add_instruction(0x40, 0x00, 0x0000, [&]() { if (m_registers.m_v[m_state.m_x] != m_state.m_nn) m_registers.m_pc += 2; });
			add_instruction(0x50, 0x00, 0x0000, [&]() { if (m_registers.m_v[m_state.m_x] == m_registers.m_v[m_state.m_y]) m_registers.m_pc += 2; });
			add_instruction(0x60, 0x00, 0x0000, [&]() { m_registers.m_v[m_state.m_x] = m_state.m_nn; });
			add_instruction(0x70, 0x00, 0x0000, [&]() { m_registers.m_v[m_state.m_x] += m_state.m_nn; });
			add_instruction(0x80, 0x00, 0x000f, [&]() { m_registers.m_v[m_state.m_x] = m_registers.m_v[m_state.m_y]; });
			add_instruction(0x80, 0x01, 0x000f, [&]() { m_registers.m_v[m_state.m_x] = m_registers.m_v[m_state.m_x] | m_registers.m_v[m_state.m_y]; if (m_flags & flags::logical_legacy) update_flag(0); });
			add_instruction(0x80, 0x02, 0x000f, [&]() { m_registers.m_v[m_state.m_x] = m_registers.m_v[m_state.m_x] & m_registers.m_v[m_state.m_y]; if (m_flags & flags::logical_legacy) update_flag(0); });
			add_instruction(0x80, 0x03, 0x000f, [&]() { m_registers.m_v[m_state.m_x] = m_registers.m_v[m_state.m_x] ^ m_registers.m_v[m_state.m_y]; if (m_flags & flags::logical_legacy) update_flag(0); });
			add_instruction(0x80, 0x04, 0x000f, [&]() 
				{
					uint16_t const sum = m_registers.m_v[m_state.m_x] + m_registers.m_v[m_state.m_y];
					m_registers.m_v[m_state.m_x] = sum & 0xff;
					update_flag(sum > 255);
				});
			add_instruction(0x80, 0x05, 0x000f, [&]()
				{
					int16_t const sub = m_registers.m_v[m_state.m_x] - m_registers.m_v[m_state.m_y];
					m_registers.m_v[m_state.m_x] = sub & 0xff;
					update_flag(sub > 0);
				});
			add_instruction(0x80, 0x06, 0x000f, [&]()
				{
					if (m_flags & flags::shift_legacy)
						m_registers.m_v[m_state.m_x] = m_registers.m_v[m_state.m_y];

					uint8_t prev = m_registers.m_v[m_state.m_x];
					m_registers.m_v[m_state.m_x] >>= 1;
					update_flag(prev & 1);
				});
			add_instruction(0x80, 0x07, 0x000f, [&]()
				{
					int16_t const sub = m_registers.m_v[m_state.m_y] - m_registers.m_v[m_state.m_x];
					m_registers.m_v[m_state.m_x] = sub & 0xff;
					update_flag(sub > 0);
				});
			add_instruction(0x80, 0x0e, 0x000f, [&]()
				{
					if (m_flags & flags::shift_legacy)
						m_registers.m_v[m_state.m_x] = m_registers.m_v[m_state.m_y];

					uint8_t prev = m_registers.m_v[m_state.m_x];
					m_registers.m_v[m_state.m_x] <<= 1;
					update_flag((prev >> 7) & 1);
				});

			add_instruction(0x90, 0x00, 0x000f, [&]() { if (m_registers.m_v[m_state.m_x] != m_registers.m_v[m_state.m_y]) m_registers.m_pc += 2; });
			add_instruction(0xa0, 0x00, 0x0000, [&]() { m_registers.m_index = m_state.m_nnn; });
			add_instruction(0xb0, 0x00, 0x0000, [&]() 
				{
					if (m_flags & flags::jump_offset_legacy)
						m_registers.m_pc = m_state.m_nnn + m_registers.m_v[0];
					else
						m_registers.m_pc = m_state.m_nnn + m_registers.m_v[m_state.m_x];
				});
			add_instruction(0xc0, 0x00, 0x0000, [&]() { m_registers.m_v[m_state.m_x] = (std::rand() % 255) & m_state.m_nn; });
			add_instruction(0xd0, 0x00, 0x0000, [&]()
				{
						auto const coordx = m_registers.m_v[m_state.m_x] & (c_displayWidth - 1);
						auto const coordy = m_registers.m_v[m_state.m_y] & (c_displayHeight - 1);
					
						uint8_t any_invalidated = 0;

						for (auto y = 0; y < m_state.m_n; ++y)
						{
							auto coordyy = (coordy + y);
							if (((m_flags & flags::draw_legacy)) && coordyy > c_displayHeight)
								continue;
							else
								coordyy &= (c_displayHeight - 1);
						
							// sprite as bit packed columns
							auto const sprite = m_memory.m_data[m_registers.m_index + y];

							// each sprite has a maximum of 8 columns
							for (auto x = 0; x < 8; ++x)
							{
								auto coordxx = (coordx + x);
								if (((m_flags & flags::draw_legacy)) && coordxx > c_displayWidth)
									continue;
								else
									coordxx &= (c_displayWidth - 1);

								// read next msb - this will be the sprite value for this column.
								auto const sprite_volumn_value = (sprite & (1 << (7 - x))) != 0;

								// extract the current value and xor it with the new, write result to display.
								auto const display_data_index = coordyy * c_displayWidth + coordxx;
								auto const previous_display_value = m_display.m_data[display_data_index];
								auto const new_display_value = (uint8_t)previous_display_value ^ (uint8_t)sprite_volumn_value;
								m_display.m_data[display_data_index] = new_display_value;

								// If the new value is off but th e previous value was on, remember this so we set vf to 1 later on.
								any_invalidated |= (previous_display_value && !new_display_value);
							}
						}

						update_flag(any_invalidated);
				});

			add_instruction(0xe0, 0x09e, 0x00ff, [&]() { if (m_input.m_key[m_registers.m_v[m_state.m_x]]) m_registers.m_pc += 2; });
			add_instruction(0xe0, 0x0a1, 0x00ff, [&]() { if (!m_input.m_key[m_registers.m_v[m_state.m_x]]) m_registers.m_pc += 2; });		

			add_instruction(0xf0, 0x07, 0x00ff, [&]() { m_registers.m_v[m_state.m_x] = m_timers.m_delay; });
			add_instruction(0xf0, 0x0a, 0x00ff, [&]()
				{
					bool keyPressed = false;
					// Check if the key is depressed
					for (uint8_t i=0; i<std::size(m_input.m_key); ++i)
					{
						if (m_input.m_key[i] ^ m_input.m_prev_key[i])
						{
							m_registers.m_v[m_state.m_x] = i;
							if (m_input.m_key[i] == 0)
							{
								m_isWaitingForInput = false;
								keyPressed = true;
							}
							break;
						}
					}

					if (!keyPressed)
						m_isWaitingForInput = true;
				});
			add_instruction(0xf0, 0x15, 0x00ff, [&]() { m_timers.m_delay = m_registers.m_v[m_state.m_x]; });
			add_instruction(0xf0, 0x18, 0x00ff, [&]() { m_timers.m_sound = m_registers.m_v[m_state.m_x]; });
			add_instruction(0xf0, 0x1e, 0x00ff, [&]() 
				{ 
					update_flag(m_registers.m_index + m_registers.m_v[m_state.m_x] > 0xfff);
					m_registers.m_index += m_registers.m_v[m_state.m_x];
				});

			add_instruction(0xf0, 0x29, 0x00ff, [&]() { m_registers.m_index += m_memory.m_font[m_state.m_x]; });
			add_instruction(0xf0, 0x33, 0x00ff, [&]()
				{
					uint8_t const v = m_registers.m_v[m_state.m_x];
					m_memory.m_data[m_registers.m_index + 0] = (v % 1000) / 100;
					m_memory.m_data[m_registers.m_index + 1] = (v % 100) / 10;
					m_memory.m_data[m_registers.m_index + 2] = (v % 10);
				});

			add_instruction(0xf0, 0x55, 0x00ff, [&]() { memcpy(&m_memory.m_data[m_registers.m_index], m_registers.m_v, sizeof(uint8_t) * (m_state.m_x + 1)); if (m_flags & flags::store_load_legacy) m_registers.m_index++; });
			add_instruction(0xf0, 0x65, 0x00ff, [&]() { memcpy(m_registers.m_v, &m_memory.m_data[m_registers.m_index], sizeof(uint8_t) * (m_state.m_x + 1)); if (m_flags & flags::store_load_legacy) m_registers.m_index++; });
		}
		
		void advance(uint8_t key_buffer[c_maxKeys])
		{		
			// Update key data and keep the previous key data around.
			memcpy(m_input.m_prev_key, m_input.m_key, sizeof(uint8_t) * c_maxKeys);
			memcpy(m_input.m_key, key_buffer, sizeof(uint8_t) * c_maxKeys);

			// Fetch, decode, execute cycle
			if (!m_isWaitingForInput)
			{
				fetch();
				decode();
			}
			execute();

			// Timers update at 60Hz
			auto const now = std::chrono::high_resolution_clock::now();
			auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_frame_end).count();
			if (elapsed_ms >= 16.66f)
			{
				if (m_timers.m_delay > 0)
					m_timers.m_delay--;

				if (m_timers.m_sound > 0)
					m_timers.m_sound--;

				m_frame_end = std::chrono::high_resolution_clock::now();
			}
		}

		// Accessors
		memory* const get_memory() { return &m_memory; }
		display* const get_display() { return &m_display; }
		registers* const get_registers() { return &m_registers; }
		timers* const get_timers() { return &m_timers; }
		input* const get_input() { return &m_input; }

	private:
		using time_point = std::chrono::high_resolution_clock::time_point;

		memory			m_memory;
		display			m_display;
		registers		m_registers;
		timers			m_timers;
		input			m_input;
		decode_state	m_state;

		decode_state	m_previousState;

		time_point		m_frame_end;
		bool			m_isWaitingForInput = false;
		flags			m_flags;

		std::unordered_map<uint8_t, instruction_family> m_families;
		instruction const* m_currentInstruction = nullptr;
		
		// Fetch the next opcode and update the decoder state.
		void fetch()
		{
			uint16_t& pc = m_registers.m_pc;

			m_previousState = m_state;

			m_state.m_opcode = (m_memory.m_data[pc] << 8) | m_memory.m_data[pc + 1];
			m_state.m_x = static_cast<uint8_t>((m_state.m_opcode >> 8) & 0x000f);
			m_state.m_y = static_cast<uint8_t>((m_state.m_opcode >> 4) & 0x000f);
			m_state.m_n = static_cast<uint8_t>(m_state.m_opcode & 0x000f);
			m_state.m_nn = static_cast<uint8_t>(m_state.m_opcode & 0x00ff);
			m_state.m_nnn = static_cast<uint16_t>(m_state.m_opcode & 0x0fff);
			
			// Advance program counter. It's fine to do this here, as very few instructions modify the counter during execution.
			pc += 2;
		}

		// Get an instruction family for a given opcode and decode the instruction.
		void decode()
		{
			uint8_t const family_key = (m_state.m_opcode & 0xf000) >> 8;
			auto const& family = m_families[family_key];

			auto const instr_it = family.m_instructions.find(m_state.m_opcode & family.m_opcodeMask);
			if (instr_it == family.m_instructions.end())
			{
				unimplemented(m_state.m_opcode);
			}

			m_currentInstruction = &instr_it->second;
		}

		void execute()
		{
			assert(m_currentInstruction != nullptr);

			printf("Pre-execute opcode: 0x%04x, pc: 0x%04x, sp: 0x%04x\n", m_state.m_opcode, m_registers.m_pc, m_registers.m_sp);

			m_currentInstruction->m_body();

			printf("Post-execute opcode: 0x%04x, pc: 0x%04x, sp: 0x%04x\n", m_state.m_opcode, m_registers.m_pc, m_registers.m_sp);			
		}

		// Add a new instruction and/or instruction family along with a callback to the instruction's body.
		template<class T>
		void add_instruction(uint8_t family_key, uint8_t instruction_key, uint16_t opcodeMask, T&& callback)
		{				
			if (!m_families.contains(family_key))
			{
				instruction_family family = { .m_opcodeMask = opcodeMask };
				m_families[family_key] = family;
			}

			auto const it = m_families.find(family_key);
			instruction const instr = { .m_body = callback, .m_family = &it->second };

			it->second.m_instructions[instruction_key] = instr;
		}

		// Update the flag register with a given value.
		void update_flag(uint8_t value)
		{
			m_registers.m_v[15] = value;
		}
	};
}


