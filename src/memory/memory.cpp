#include <fstream>
#include <format>

#include <memory/memory.hpp>
#include <spdlog/spdlog.h>

memory::Memory::Memory()
{
    // nothing to do
}

memory::Memory::~Memory()
{
    this->memory->clear();
    delete this->memory;
}

void memory::Memory::init()
{
    spdlog::info("allocating memory for chip-8");
    this->memory = new std::vector<std::byte>(MEM_SIZE);
}

void memory::Memory::load_font(font::Font *font_data)
{
    for (std::size_t i = 0; i < FONT_DATA_SIZE; i++)
    {
        this->memory->at(FONT_START_AT + i) = font_data->data()->at(i);
    }
}

void memory::Memory::load_program(std::string rom_file_name)
{
    spdlog::info("loading rom {}", rom_file_name);
    // file exists already checked
    std::ifstream rom_file;
    rom_file.open(rom_file_name, std::ios::binary);
    // parse byte by byte
    rom_file.seekg(0, std::ios::end);
    std::streampos size = rom_file.tellg();
    rom_file.seekg(0, std::ios::beg);
    rom_file.read(reinterpret_cast<char *>(this->memory->data()) + ROM_START_AT, size);

    if (rom_file.gcount() == 0)
    {
        throw std::runtime_error(std::format("failed to load rom: {}", rom_file_name));
    }

    // this->view_memory(0, MEM_SIZE);
}

void memory::Memory::view_memory(mem_addr offset, size_t length)
{
    if (offset > MEM_SIZE || offset + length > MEM_SIZE)
    {
        throw std::runtime_error(std::format("index {} out of range when viewing memory chunk", offset));
    }

    spdlog::set_pattern("mem %v");
    for (mem_addr i = offset; i < offset + length; i++)
    {
        spdlog::info("{:4}    0x{:<x}", i, this->read(i));
    }
    spdlog::set_pattern("%+");
}

std::byte memory::Memory::read(mem_addr addr)
{
    return this->memory->at(addr);
}

void memory::Memory::write(mem_addr addr, std::byte data)
{
    this->memory->at(addr) = data;
}