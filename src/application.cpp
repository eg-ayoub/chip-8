#include <fstream>
#include <format>
#include <algorithm>
#include <chrono>
#include <thread>

#include <application.hpp>
#include <spdlog/spdlog.h>

application::Application::Application(int clock, std::string rom, std::string font)
{
    this->clock = clock;
    this->rom_file_name = rom;

    // check that rom exists
    spdlog::info("checking rom file");
    std::ifstream romfile;
    romfile.open(rom);
    if (not romfile)
    {
        romfile.close();
        throw std::runtime_error(std::format("unable to load rom: {}", rom));
    }
    romfile.close();

    // check that the font file exists
    spdlog::info("checking font file");
    if (font != "nofont")
    {
        std::ifstream fontfile;
        fontfile.open(font);
        if (not fontfile)
        {
            spdlog::warn("unable to load font: {}", font);
            spdlog::info("reverting to default font");
            font = "nofont";
        }
        fontfile.close();
    }

    // instantiate components

    // * font
    spdlog::info("creating font object");
    if (font != "nofont")
    {
        this->font = new font::Font(font);
    }
    else
    {
        this->font = new font::Font();
    }

    // * memory
    spdlog::info("creating memory object");
    this->ram = new memory::Memory();

    // * registers
    spdlog::info("creating registers");
    this->V = new std::vector<reg::register_t>(REGISTER_COUNT);

    // * stack
    spdlog::info("creating stack");
    this->stack = new stack::Stack();

    // * display
    spdlog::info("initializing SDL");
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        throw std::runtime_error(std::format("unable to init SDL: {}", SDL_GetError()));
    }

    spdlog::info("creating SDL window");
    this->window = SDL_CreateWindow("CHIP-8", DISPLAY_WIDTH * PIXEL_SIZE, DISPLAY_HEIGHT * PIXEL_SIZE, 0);
    if (this->window == NULL)
    {
        throw std::runtime_error(std::format("unable to init SDL window: {}", SDL_GetError()));
    }

    spdlog::info("creating display object");
    this->display = new display::Display(this->window);
}

application::Application::~Application()
{
    // cleanup components
    this->cleanup();
}

void application::Application::init()
{
    // init components

    // * memory
    spdlog::info("initializing memory component");
    this->ram->init();

    spdlog::info("loading font data into memory");
    this->ram->load_font(this->font);

    spdlog::info("loading rom file into memory");
    this->ram->load_program(this->rom_file_name);

    // * stack
    spdlog::info("initializing stack");
    this->stack->init();

    // * registers
    spdlog::info("setting registers to 0");
    std::fill(this->V->begin(), this->V->end(), (std::byte)0);

    // * timers
    spdlog::info("setting timers to 0");
    this->delay_timer = 0;
    this->sound_timer = 0;

    // * PC
    spdlog::info("aligning pc to 0x{:x}", ROM_START_AT);
    this->PC = ROM_START_AT;

    // * I
    spdlog::info("setting memory index to 0");
    this->I = 0;

    // * display
    spdlog::info("initializing display");
    this->display->init();
}

void application::Application::run()
{
    std::thread timers(&Application::timers_thread, this);
    std::thread main(&Application::main_thread, this);

    // this should end on its own
    main.join();

    // force end the timers thread
    this->stop_timers_thread = true;
    timers.join();
}

void application::Application::cleanup()
{
    // * memory
    spdlog::info("cleaning up memory component");
    delete this->ram;

    spdlog::info("cleaning up font object");
    delete this->font;

    // * registers
    spdlog::info("cleaning up registers");
    delete this->V;

    // * stack
    spdlog::info("cleaning up stack");
    delete this->stack;

    // * keypad

    // * display
    spdlog::info("cleaning up display");
    delete this->display;

    spdlog::info("destroying sdl window");
    SDL_DestroyWindow(this->window);
    this->window = NULL;
    SDL_Quit();
}

void application::Application::timers_thread()
{
    this->stop_timers_thread = false;
    while (not this->stop_timers_thread)
    {
        // wait 1/60 second
        // std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1, TIMER_CLOCK>>(1));

        if (this->delay_timer != 0)
        {
            this->delay_timer--;
        }

        if (this->sound_timer != 0)
        {
            this->sound_timer--;
        }
    }
}

void application::Application::main_thread()
{
    int c = 0;
    while (c < 5)
    {
        if (this->stop_timers_thread)
        {
            // something bad happened in the other thread
            spdlog::warn("something bad happened to the timer thread");
            break;
        }
        this->display->draw(0, 0, new std::vector<std::byte>({std::byte{0b01010101}}));
        this->display->update();
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        SDL_Delay(1000);
        c++;
    }
}
