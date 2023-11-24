#include "../utility/Clock.hpp"
#include "../utility/File.hpp"
#include "../utility/Random.hpp"
#include "../utility/log.hpp"
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <SDL_timer.h>
#include <SDL_video.h>
#include <bits/stdc++.h>
#include <bitset>
#include <chrono>
#include <cstdlib>
#include <map>
#include <random>

inline uint8_t flip_byte(uint8_t c)
{
    uint8_t r = 0;
    for (uint8_t i = 0; i < 8; i++) {
        r <<= 1;
        r |= c & 1;
        c >>= 1;
    }
    return r;
}

class Chip8 {
public:
    static std::optional<Chip8> from_file(std::string_view filename)
    {
        auto maybe_file = File::open(filename);
        if (!maybe_file.has_value()) {
            logln("file doesn't exist");
            return {};
        }
        auto program_bytes = (*maybe_file)->read_all();
        return Chip8 { program_bytes };
    }

    bool step()
    {
        if (key_press_register.has_value())
            return false;

        if (m_delay_timer > 0) {
            m_delay_timer--;
            logln("Delay timer: {}", m_delay_timer);
        }

        if (m_sound_timer > 0) {
            m_sound_timer--;
            logln("Sound timer: {}", m_sound_timer);
            logln("BZZZ");
        }

        auto opcode = m_system_memory.slice_span(m_program_counter, 2);
        auto first_byte = opcode[0];
        auto second_byte = opcode[1];

        /*printf("PC: 0x%04X\n", m_program_counter);
        printf("OP: 0x%04X\n", first_byte << 8 | second_byte);
        printf("STACK: ");
        for (size_t i = 0; i < m_stack_pointer; i++) {
            printf("0x%04X  ", m_stack[i]);
        }
        printf("\n");

        for (size_t i = 0; i < m_registers.size(); i++) {
            printf("V%1zX: 0x%02X  ", i, m_registers[i]);
        }
        printf("\n");

        printf("I: 0x%03X\n", m_i_register);*/

        if (first_byte == 0x00 && second_byte == 0xee) {
            // Return
            // printf("Address: 0x%02X return", m_program_counter);
            m_stack_pointer--;
            m_program_counter = m_stack[m_stack_pointer];
            // printf("ing to 0x%04X\n", m_program_counter);
        } else if (first_byte == 0x00 && second_byte == 0xe0) {
            // Display
            // printf("Address: 0x%02X clear display\n", m_program_counter);
            m_display.fill(0);
            m_program_counter += 2;
        } else if ((first_byte & 0xf0) == 0x10) {
            // GOTO
            uint16_t address = ((first_byte << 8) | second_byte) & 0xfff;
            if (address == m_program_counter) {
                printf("Program complete. Goodbye! :)\n");
                // TODO: pause the program instead of quitting
                return true;
            }
            // printf("Address: 0x%02X Goto %03X\n", m_program_counter, address);
            m_program_counter = address;
        } else if ((first_byte & 0xf0) == 0x20) {
            // Flow - subroutine call
            uint16_t address = ((first_byte << 8) | second_byte) & 0xfff;
            // printf("Address: 0x%02X Call subroutine at %03X\n", m_program_counter, address);
            m_program_counter += 2;
            m_stack[m_stack_pointer] = m_program_counter;
            m_stack_pointer++;
            m_program_counter = address;
        } else if ((first_byte & 0xf0) == 0x30) {
            // Cond - Skip the next instruction if VX == NN
            if (m_registers[first_byte & 0x0f] == second_byte) {
                // printf("Address: 0x%02X V%1X == %02X, skipping\n", m_program_counter, first_byte & 0x0f, second_byte);
                m_program_counter += 4;
            } else {
                //("Address: 0x%02X V%1X != %02X, not skipping\n", m_program_counter, first_byte & 0x0f, second_byte);
                m_program_counter += 2;
            }
        } else if ((first_byte & 0xf0) == 0x40) {
            // Cond - Skip the next instruction if VX != NN
            if (m_registers[first_byte & 0x0f] != second_byte) {
                // printf("Address: 0x%02X V%1X != %02X, skipping\n", m_program_counter, first_byte & 0x0f, second_byte);
                m_program_counter += 4;
            } else {
                // printf("Address: 0x%02X V%1X == %02X, not skipping\n", m_program_counter, first_byte & 0x0f, second_byte);
                m_program_counter += 2;
            }
        } else if ((first_byte & 0xf0) == 0x60) {
            // Const - set register
            // printf("Address: 0x%02X V%1X = %02X\n", m_program_counter, first_byte & 0xf, second_byte);
            m_registers[first_byte & 0x0f] = second_byte;
            m_program_counter += 2;
        } else if ((first_byte & 0xf0) == 0x70) {
            // Add NN to Vx (carry flag is not changed)
            // printf("Address: 0x%02X V%1X += %02X\n", m_program_counter, first_byte & 0xf, second_byte);
            // FIXME: Intentional overflow, can we check this?
            m_registers[first_byte & 0xf] += second_byte;
            m_program_counter += 2;
        } else if ((first_byte & 0xf0) == 0xa0) {
            // MEM - set I register
            m_i_register = ((first_byte << 8) | second_byte) & 0xfff;
            // printf("Address: 0x%02X I = %03X\n", m_program_counter, m_i_register);
            m_program_counter += 2;
        } else if ((first_byte & 0xf0) == 0xc0) {
            // Random, vX = rand() & NN
            auto rand = m_random.take_int<uint8_t>();
            m_registers[first_byte & 0xf] = rand & second_byte;
            // printf("Address: 0x%02X V%1X = %02X (rand) & %02X = %02X\n", m_program_counter, first_byte & 0xf, rand, second_byte, m_registers[first_byte & 0xf]);

            m_program_counter += 2;
        } else if ((first_byte & 0xf0) == 0xd0) {
            // Display
            //  draw sprite at coordinate (vx, vy) that has a width of 8 pixels and height of N
            // thus it will always draw N rows in the same column (which may be offset by some M bits)
            auto height = second_byte & 0xf;

            auto x_origin = m_registers[first_byte & 0xf];
            auto y_origin = m_registers[second_byte >> 4];
            //  read N*8 bytes from I address
            auto pixels = m_system_memory.slice_span(m_i_register, height);

            //  each pixel is bit-coded, e.g. 1s = active pixel, 0s = dead pixel
            //  pixels are XORed to the screen, if an active pixel is XORed set the carry flag to 1
            m_registers[0xf] = 0;

            auto remainder = x_origin % 8;
            for (int j = 0; j < height; j++) {
                if (remainder != 0) {
                    // To handle remainders we have to write across two bytes
                    // Get the index of each byte
                    auto first_index = floor(x_origin / 8) + 8 * (j + y_origin);
                    auto second_index = first_index + 1;

                    // Check the amount of set bits to determine if any have changed by the end
                    auto first_set_bits = std::bitset<8>(m_display[first_index]).count();
                    auto second_set_bits = std::bitset<8>(m_display[second_index]).count();

                    // Both bytes are combined into a single variable
                    uint16_t display = m_display[first_index] << 8 | m_display[second_index];
                    // The bytes to be written are shifted to the correct bit-index, e.g. a remainder of 5 will start from the 5th bit of the first byte
                    display ^= pixels[j] << remainder;
                    // Unpack each byte from this variable
                    m_display[first_index] = display >> 8;
                    m_display[second_index] = display & 0xff;

                    if (first_set_bits != std::bitset<8>(m_display[first_index]).count() || second_set_bits != std::bitset<8>(m_display[second_index]).count()) {
                        m_registers[0xf] = 1;
                    }
                } else {
                auto index = floor(x_origin / 8) + 8 * (j + y_origin);
                auto set_bits = std::bitset<8>(m_display[index]).count();
                    m_display[index] ^= pixels[j];

                if (set_bits != std::bitset<8>(m_display[index]).count())
                    m_registers[0xf] = 1;
                }
            }

            m_program_counter += 2;
        } else if ((first_byte & 0xf0) == 0xf0 && second_byte == 0x7) {
            // Get delay timer
            m_registers[first_byte & 0xf] = m_delay_timer;
            m_program_counter += 2;
        } else if ((first_byte & 0xf0) == 0xf0 && (second_byte & 0xf) == 0xa) {
            // KeyOp - wait for key event
            if (!key_press_register.has_value()) {
                // printf("Address: 0x%02X V%1X; Waiting for key press 0-9a-f\n", m_program_counter, first_byte & 0xf);
                printf("Waiting for key...\n");
                key_press_register = first_byte & 0xf;
                // registers[key_press_register] = key in press_key
            }
        } else if ((first_byte & 0xf0) == 0xf0 && second_byte == 0x15) {
            m_delay_timer = m_registers[first_byte & 0xf];
            m_program_counter += 2;
        } else if ((first_byte & 0xf0) == 0xf0 && second_byte == 0x1e) {
            m_i_register += m_registers[first_byte & 0xf];
            m_program_counter += 2;
        } else {
            printf("Address: 0x%02X unhandled opcode: %02X%02X noop\n", m_program_counter, first_byte, second_byte);
            m_program_counter += 2;
        }

        return false;
    }

    void press_key(uint8_t key)
    {
        if (!key_press_register.has_value()) {
            logln("keypress unhandled, program not ready");
            return;
        }
        printf("Received key press %1X\n", key);
        m_registers[*key_press_register] = key;
        m_program_counter += 2;
        key_press_register = {};
    }

    std::array<uint8_t, 256> display()
    {
        return m_display;
    }

    void display_test_pattern()
    {
        // clang-format off
        m_display = {
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
            0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00,
        };
        // clang-format on
    }

private:
    Chip8(Bytes const& program_bytes)
    {
        m_system_memory.zero();
        m_system_memory.overwrite(m_program_counter, program_bytes);
    }

    Random m_random = {};

    Bytes m_system_memory = { 4096 };
    std::array<uint16_t, 64> m_stack = {};
    size_t m_stack_pointer = 0;
    uint16_t m_program_counter = 0x200;
    uint16_t m_i_register = 0;
    uint8_t m_delay_timer = 0;
    uint8_t m_sound_timer = 0;
    std::array<uint8_t, 0x10> m_registers = {};
    std::array<uint8_t, 256> m_display = {};

    std::optional<uint8_t> key_press_register;
};

int main(int argc, char* argv[])
{
    if (argc != 2) {
        logln("Provide a filename");
        return EXIT_FAILURE;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        logln("SDL failed to start: {}", SDL_GetError());
        return EXIT_FAILURE;
    }

    auto maybe_chip8 = Chip8::from_file(argv[1]);

    if (!maybe_chip8.has_value()) {
        return EXIT_FAILURE;
    }

    SDL_Window* window;
    SDL_Renderer* renderer;
    int render_scale = 10;
    SDL_CreateWindowAndRenderer(64 * render_scale, 32 * render_scale, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    bool close = false;

    auto accumulator = 0;
    auto last_update = SDL_GetTicks();
    auto logic = 0;
    TTF_Init();
    TTF_Font* font = TTF_OpenFont("roboto.ttf", 12);
    if (!font)
        std::cout << "Couldn't find/init open ttf font." << std::endl;
    SDL_Texture* text_texture;
    SDL_Rect text_rect;
    std::string key_text;
    while (!close) {
        auto delta = SDL_GetTicks() - last_update;
        accumulator += delta;
        last_update = SDL_GetTicks();

        while (accumulator > 16) {
            // logic step once
            logic += 1;
            maybe_chip8->step();
            accumulator -= 16;
        }

        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                close = true;
                break;
            case SDL_KEYDOWN:
                int scancode = event.key.keysym.scancode;

                if (scancode == SDL_SCANCODE_Q || scancode == SDL_SCANCODE_ESCAPE) {
                    close = true;
                } else if (scancode == SDL_SCANCODE_RETURN) {
                    maybe_chip8->display_test_pattern();
                } else if (scancode == SDL_SCANCODE_0) {
                    maybe_chip8->press_key(0);
                    key_text = "0";
                } else if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9) {
                    maybe_chip8->press_key(scancode - SDL_SCANCODE_1 + 1);
                    key_text = std::to_string(scancode - SDL_SCANCODE_1 + 1);
                } else if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_F) {
                    maybe_chip8->press_key(scancode - SDL_SCANCODE_A + 10);
                    key_text = std::to_string(scancode - SDL_SCANCODE_A + 10);
                } else {
                    // Other key presses are not handled
                }
            }
        }

        {
            /*Random random;

            for (int i = 0; i < 256; i++) {
                auto col = i % 8;
                int row = floor(i / 8);
                SDL_SetRenderDrawColor(renderer, random.take_int<uint8_t>(), random.take_int<uint8_t>(), random.take_int<uint8_t>(), 255);
                for (int j = 0; j < 8; j++) {
                    if (render_scale == 1)
                        SDL_RenderDrawPoint(renderer, col * 8 + j, row);
                    else {
                        SDL_Rect rect { (col * 8 + j) * render_scale, row * render_scale, render_scale, render_scale };
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }*/
        }

        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            int index = 0;
            for (auto byte : maybe_chip8->display()) {
                auto col = index % 8;
                int row = floor(index / 8);

                /*
                Each byte represents an 8-bit section of the image, thus we need 32 rows of 8 bytes
                 ________________________________________________________________________
                |00000000|00000000|00000000|00000000|00000000|00000000|00000000|00000000| 8 bytes (8x8bits=64)
                |00000000|0...
                | <30 more rows>
                */
                for (int i = 0; i < 8; i++) {
                    if (byte & (1 << i)) {
                        if (render_scale == 1)
                            SDL_RenderDrawPoint(renderer, col * 8 + (7 - i), row);
                        else {
                            SDL_Rect rect { (col * 8 + (7 - i)) * render_scale, row * render_scale, render_scale, render_scale };
                            SDL_RenderFillRect(renderer, &rect);
                        }
                    }
                }

                index++;
            }
        }

        {
            std::string text = "logic: " + std::to_string(logic);
            SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), { 255, 255, 255, 255 });
            text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            text_rect.x = 0; // Center horizontaly
            text_rect.y = 0; // Center verticaly
            text_rect.w = text_surface->w;
            text_rect.h = text_surface->h;
            // After you create the texture you can release the surface memory allocation because we actually render the texture not the surface.
            SDL_FreeSurface(text_surface);
            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
            SDL_DestroyTexture(text_texture);
        }
        {
            std::string text = "key: " + key_text;
            SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), { 255, 255, 255, 255 });
            text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            text_rect.x = 0; // Center horizontaly
            text_rect.y = text_surface->h; // Center verticaly
            text_rect.w = text_surface->w;
            text_rect.h = text_surface->h;
            // After you create the texture you can release the surface memory allocation because we actually render the texture not the surface.
            SDL_FreeSurface(text_surface);
            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
            SDL_DestroyTexture(text_texture);
        }

        SDL_RenderPresent(renderer);
    }

    TTF_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
