#include <format>
#include <exception>

#include <display/display.hpp>

#include <spdlog/spdlog.h>


std::string display::Pixel::toString()
{
    return std::format("{}({}=>{}, {}=>{})", state ? "ON" : "OFF", rect.x, rect.x + rect.w, rect.y, rect.y + rect.h);
}

display::Display::Display(SDL_Window *window)
{
    this->window = window;
    this->renderer = NULL;
    this->bgcol = {0x81, 0xBE, 0xCE, SDL_ALPHA_OPAQUE};
    this->fgcol = {0x01, 0x2F, 0x4A, SDL_ALPHA_OPAQUE};
}

display::Display::~Display()
{
    SDL_DestroyRenderer(this->renderer);
    delete this->pixels;
}

void display::Display::init()
{
    spdlog::info("getting SDL Renderer");
    this->renderer = SDL_CreateRenderer(this->window, NULL, SDL_RENDERER_ACCELERATED);
    if (this->renderer == NULL)
    {
        throw std::runtime_error(std::format("unable to init SDL renderer: {}", SDL_GetError()));
    }

    spdlog::info("populating display pixels");
    this->pixels = new std::vector<Pixel>(DISPLAY_HEIGHT * DISPLAY_WIDTH);
    for (size_t x = 0; x < DISPLAY_WIDTH; x++)
    {
        for (size_t y = 0; y < DISPLAY_HEIGHT; y++)
        {
            size_t p = index(x, y);
            this->pixels->at(p).state = false;
            this->pixels->at(p).rect.x = x * PIXEL_SIZE;
            this->pixels->at(p).rect.y = y * PIXEL_SIZE;
            this->pixels->at(p).rect.w = PIXEL_SIZE;
            this->pixels->at(p).rect.h = PIXEL_SIZE;
        }
    }
    this->update();
}

void display::Display::clear()
{
    spdlog::trace("clearing screen");
    for (size_t x = 0; x < DISPLAY_WIDTH; x++)
    {
        for (size_t y = 0; y < DISPLAY_HEIGHT; y++)
        {
            size_t p = index(x, y);
            this->pixels->at(p).state = false;
        }
    }
}

void display::Display::update()
{
    spdlog::trace("update window");
    // * clear screen
    SDL_SetRenderDrawColor(this->renderer, this->bgcol.r, this->bgcol.g, this->bgcol.b, this->bgcol.a);
    SDL_RenderClear(this->renderer);

    // * draw pixels one by one
    SDL_SetRenderDrawColor(this->renderer, this->fgcol.r, this->fgcol.g, this->fgcol.b, this->fgcol.a);
    for (size_t x = 0; x < DISPLAY_WIDTH; x++)
    {
        for (size_t y = 0; y < DISPLAY_HEIGHT; y++)
        {
            size_t p = index(x, y);
            if (this->pixels->at(p).state)
            {
                SDL_SetRenderDrawColor(this->renderer, this->fgcol.r, this->fgcol.g, this->fgcol.b, this->fgcol.a);
                // spdlog::info(this->pixels->at(p).toString());
                SDL_RenderFillRect(this->renderer, &(this->pixels->at(p).rect));
                // SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, this->fgcol.a);
                // SDL_RenderRect(this->renderer, &(this->pixels->at(p).rect));

            }
        }
    }

    // * present
    SDL_RenderPresent(this->renderer);
    SDL_Delay(0);
}

int display::Display::draw(size_t x, size_t y, std::vector<std::byte> *sprite)
{
    int ret = 0;
    // wrap screen
    x = x % DISPLAY_WIDTH;
    y = y % DISPLAY_HEIGHT;
    // we draw from (x, y) to (x + 8, y + LEN)
    for (size_t rel_x = 0; rel_x < 8; rel_x++)
    {
        for (size_t rel_y = 0; rel_y < sprite->size(); rel_y++)
        {
            if ((sprite->at(rel_y) & FLAGS[7 - rel_x]) != std::byte{0})
            {
                size_t p = index(x + rel_x, y + rel_y);
                if (p < this->pixels->size())
                {
                    this->pixels->at(p).state = not this->pixels->at(p).state;
                    if (not this->pixels->at(p).state)
                    {
                        // a pixel was turned off
                        ret = 1;
                    }
                }
            }
        }
    }
    return ret;
}
