#pragma once

#include <iostream>
#include <exception>
#include <atomic>

#include <SDL3/SDL.h>

#include <memory/memory.hpp>
#include <font/font.hpp>
#include <timer/timer.hpp>
#include <reg/reg.hpp>
#include <stack/stack.hpp>
#include <display/display.hpp>

#ifndef REGISTER_COUNT
#define REGISTER_COUNT 16
#endif

#ifndef TIMER_CLOCK
#define TIMER_CLOCK 60
#endif

namespace application
{
    const std::byte FIRST_NIBBLE = std::byte{0xF0};
    const std::byte SECOND_NIBBLE = std::byte{0x0F};

    class Application
    {
    private:
        uint clock;
        std::string rom_file_name;

        SDL_Window *window;

        font::Font *font;
        memory::Memory *ram;
        stack::Stack *stack;
        display::Display *display;

        std::vector<reg::register_t> *V; // registers

        timer::timer_t delay_timer;
        timer::timer_t sound_timer;

        memory::mem_addr PC; // Program Counter
        memory::mem_addr I;  // Index

        std::atomic<bool> stop_timers_thread;

    public:
        Application(uint clock, std::string rom, std::string font);
        ~Application();
        void init();
        void run();
        void cleanup();
        void timers_thread();
        void interpret(std::byte n12, std::byte n34);
    };
}
