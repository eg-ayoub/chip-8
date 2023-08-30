#include <exception>

#include <keypad/keypad.hpp>

#include <spdlog/spdlog.h>

keypad::Keypad::Keypad()
{
    // nothing here
}

keypad::Keypad::~Keypad()
{
    delete this->keys;
}

void keypad::Keypad::init()
{
    this->keys = new std::vector<bool>(16);
    this->reset();
}

void keypad::Keypad::reset()
{
    for(size_t key = 0; key < 16; key++)
    {
        this->keys->at(key) = false;
    }
}

void keypad::Keypad::register_key(SDL_Scancode scancode)
{
    for(size_t key = 0; key < 16; key++)
    {
        if (scancode == SCANCODES[key])
        {
            this->keys->at(key) = true;
            return;
        }
    }
    spdlog::warn("invalid key pressed {}", (int)scancode);
}

void keypad::Keypad::release_key(SDL_Scancode scancode)
{
    for(size_t key = 0; key < 16; key++)
    {
        if (scancode == SCANCODES[key])
        {
            this->keys->at(key) = false;
            return;
        }
    }
    spdlog::warn("invalid key released {}", (int)scancode);
}

bool keypad::Keypad::is_pressed(uint8_t key)
{
    return this->keys->at(key);
}

uint8_t keypad::Keypad::wait_for_key()
{
    SDL_Event e;
    while (1)
    {
        while (SDL_PollEvent(&e) == 0)
        {
            // do nothing
            // so this won't hang
            SDL_Delay(0);
        }
        if (e.type == SDL_EVENT_KEY_DOWN)
        {
            for(size_t key = 0; key < 16; key++)
            {
                if (e.key.keysym.scancode == SCANCODES[key])
                {
                    return key;
                }
            }
        }
    }
}