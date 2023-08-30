#pragma once

#include <vector>

#include <SDL3/SDL.h>

namespace keypad
{

    const SDL_Scancode SCANCODES[16] = {
        SDL_SCANCODE_X, // 0
        SDL_SCANCODE_1, // 1
        SDL_SCANCODE_2, // 2
        SDL_SCANCODE_3, // 3

        SDL_SCANCODE_Q, // 4
        SDL_SCANCODE_W, // 5
        SDL_SCANCODE_E, // 6
        SDL_SCANCODE_A, // 7

        SDL_SCANCODE_S, // 8
        SDL_SCANCODE_D, // 9
        SDL_SCANCODE_Z, // A
        SDL_SCANCODE_C, // B

        SDL_SCANCODE_4, // C
        SDL_SCANCODE_R, // D
        SDL_SCANCODE_F, // E
        SDL_SCANCODE_V  // F
    };

    class Keypad
    {
        private:
            std::vector<bool> *keys;
        public:
            Keypad();
            ~Keypad();
            void init();
            void reset();
            void register_key(SDL_Scancode scancode);
            void release_key(SDL_Scancode scancode);
            bool is_pressed(uint8_t key);
            uint8_t wait_for_key();
    };
}