#pragma once

#include <iostream>
#include <exception>
#include <atomic>

#include <memory/memory.hpp>
#include <font/font.hpp>
#include <timer/timer.hpp>
#include <reg/reg.hpp>
#include <stack/stack.hpp>

#ifndef REGISTER_COUNT
#define REGISTER_COUNT 16
#endif

namespace application
{
    class Application
    {
    private:
        int clock;
        std::string rom_file_name;

        font::Font *font;
        memory::Memory *ram;
        stack::Stack *stack;

        std::vector<reg::register_t> *V; // registers

        timer::timer_t delay_timer;
        timer::timer_t sound_timer;

        memory::mem_addr PC; // Program Counter
        memory::mem_addr I;  // Index

        std::atomic<bool> stop_timers_thread;

    public:
        Application(int clock, std::string rom, std::string font);
        ~Application();
        void init();
        void run();
        void cleanup();
        void timers_thread();
        void main_thread();
    };
}
