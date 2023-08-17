#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#include <font/font.hpp>

#ifndef ROM_START_AT
#define ROM_START_AT 0x200u
#endif

#ifndef FONT_START_AT
#define FONT_START_AT 0x50u
#endif

#ifndef MEM_SIZE
#define MEM_SIZE 4096u
#endif

namespace memory
{

    typedef uint16_t mem_addr;

    class Memory
    { // 4KB
    private:
        std::vector<std::byte> *memory;

    public:
        Memory();
        ~Memory();
        void init();
        void load_font(font::Font *font_data);
        void load_program(std::string rom_file_name);
        void view_memory(mem_addr offset, size_t length);
        std::byte read(mem_addr addr);
        void write(mem_addr addr, std::byte data);
    };
}
