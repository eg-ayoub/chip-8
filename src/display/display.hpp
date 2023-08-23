#pragma once

#include <vector>
#include <format>

#include <SDL3/SDL.h>

#ifndef DISPLAY_WIDTH
#define DISPLAY_WIDTH 64
#endif

#ifndef DISPLAY_HEIGHT
#define DISPLAY_HEIGHT 32
#endif

#ifndef PIXEL_SIZE
#define PIXEL_SIZE 20
#endif

namespace display
{

    inline size_t index(size_t x, size_t y) { return DISPLAY_WIDTH * y + x; };

    const std::byte FLAGS[8] = {
        std::byte{0b00000001},
        std::byte{0b00000010},
        std::byte{0b00000100},
        std::byte{0b00001000},
        std::byte{0b00010000},
        std::byte{0b00100000},
        std::byte{0b01000000},
        std::byte{0b10000000}};

    struct Pixel
    {
        bool state;
        SDL_FRect rect;

        std::string toString();
    };

    class Display
    {
    private:
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Color bgcol;
        SDL_Color fgcol;
        std::vector<Pixel> *pixels;

    public:
        Display(SDL_Window *window);
        ~Display();
        void init();
        void clear();
        int draw(size_t x, size_t y, std::vector<std::byte> *sprite);
        void update();
    };
}