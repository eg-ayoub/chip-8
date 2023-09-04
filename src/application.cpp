#include <fstream>
#include <format>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cstdlib>

#include <application.hpp>
#include <spdlog/spdlog.h>

application::Application::Application(uint clock, std::string rom, std::string font)
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
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

    // * keypad
    spdlog::info("creating keypad object");
    this->keypad = new keypad::Keypad();

    // * beeper
    spdlog::info("creating beeper object");
    this->beeper = new beep::Beeper();
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
    std::fill(this->V->begin(), this->V->end(), std::byte{0});

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

    // * keypad
    spdlog::info("initializing keypad");
    this->keypad->init();

    // * beeper
    spdlog::info("initializing beeper");
    this->beeper->init();
}

void application::Application::run()
{
    uint32_t exec_delay_ms = 1000 / this->clock;
    uint32_t start = 0;
    uint32_t elapsed = 0;

    std::thread timers(&Application::timers_thread, this);
    try {
        bool quit = false;
        SDL_Event e;
        while (not quit)
        {
            start = SDL_GetTicks();
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_EVENT_QUIT)
                {
                    quit = true;
                }
                if (e.type == SDL_EVENT_KEY_DOWN)
                {
                    // handle keys
                    this->keypad->register_key(e.key.keysym.scancode);
                }
                if (e.type == SDL_EVENT_KEY_UP)
                {
                    // handle keys
                    this->keypad->release_key(e.key.keysym.scancode);
                }
            }
            if (this->stop_timers_thread)
            {
                // something bad happened in the other thread
                spdlog::warn("something bad happened to the timer thread");
                break;
            }
            // * execution loop
            // * fetch
            // * first and second nibbles
            std::byte n1_n2 = this->ram->read(this->PC);
            this->PC++;
            // * third and fourth nibbles
            std::byte n3_n4 = this->ram->read(this->PC);
            this->PC++;
            // * decode and exec
            this->interpret(n1_n2, n3_n4);
            // * loop
            elapsed = SDL_GetTicks() - start;
            if (elapsed < exec_delay_ms)
            {
                SDL_Delay(exec_delay_ms - elapsed);
            }
        }
    } catch (std::runtime_error &e)
    {
        spdlog::info("terminate timers thread");
        this->stop_timers_thread = true;
        timers.join();
        throw e;
    }

    // force end the timers thread
    spdlog::info("terminate timers thread");
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
    spdlog::info("cleaning up keypad");
    delete this->keypad;

    // * beeper
    spdlog::info("cleaning up beeper");
    delete this->beeper;

    // * display
    spdlog::info("cleaning up display");
    delete this->display;
    
    spdlog::info("destroying sdl mixer");
    Mix_Quit();

    spdlog::info("destroying sdl window");
    SDL_DestroyWindow(this->window);
    this->window = NULL;
    SDL_Quit();
}

void application::Application::timers_thread()
{
    this->stop_timers_thread = false;
    this->delay_timer = 0;
    this->sound_timer = 0;
    while (not this->stop_timers_thread)
    {
        // * wait 1/60 second
        std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1, TIMER_CLOCK>>(1));

        if (this->delay_timer != 0)
        {
            this->delay_timer--;
        }

        if (this->sound_timer != 0)
        {
            this->beeper->start();
            this->sound_timer--;
        }
        else {
            this->beeper->stop();
        }
    }
}

void application::Application::interpret(std::byte n12, std::byte n34)
{
    uint8_t vx, vy, X, Y, N, result;
    memory::mem_addr to, index;
    std::vector<std::byte> *sprite;
    // spdlog::info("INST 0x{:02X}{:02X}", n12, n34);
    switch (n12 & FIRST_NIBBLE)
    {
        case std::byte{0x00}:
            // ? O???
            // second nibble of first byte is only used
            // for 0NNN which won't be impl
            switch (n34)
            {
                case std::byte{0xEE}:
                    // return from subroutine
                    to = this->stack->pop();
                    this->PC=to;
                    break;
                case std::byte{0xE0}:
                    // clear screen
                    this->display->clear();
                    this->display->update();
                    break;
            }
            break;
        case std::byte{0x10}:
            // jump
            to = (memory::mem_addr)(n12 & SECOND_NIBBLE) << 8 | (memory::mem_addr)n34;
            this->PC = to;
            break;
        case std::byte{0x20}:
            // go to subroutine
            this->stack->push(this->PC);
            to = (memory::mem_addr)(n12 & SECOND_NIBBLE) << 8 | (memory::mem_addr)n34;
            this->PC = to;
            break;
        case std::byte{0x30}:
            // skip if VX == NN
            vx = (uint8_t)(n12 & SECOND_NIBBLE);
            if (this->V->at(vx) == n34)
            {
                this->PC += 2;
            }
            break;
        case std::byte{0x40}:
            // skip if VX != NN
            vx = (uint8_t)(n12 & SECOND_NIBBLE);
            if (this->V->at(vx) != n34)
            {
                this->PC += 2;
            }
            break;
        case std::byte{0x50}:
            // skip if VX == VY
            vx = (uint8_t)(n12 & SECOND_NIBBLE);
            vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
            if (this->V->at(vx) == this->V->at(vy))
            {
                this->PC += 2;
            }
            break;
        case std::byte{0x60}:
            // set VX = NN
            vx = (uint8_t)(n12 & SECOND_NIBBLE);
            this->V->at(vx) = n34;
            break;
        case std::byte{0x70}:
            // set VX = VX + NN
            vx = (uint8_t)(n12 & SECOND_NIBBLE);
            result = (uint8_t)(std::byte{this->V->at(vx)}) + (uint8_t)n34;
            this->V->at(vx) = std::byte{result};
            break;
        case std::byte{0x80}:
            // decode accorfing to second nibble of second byte
            switch (n34 & SECOND_NIBBLE)
            {
                case std::byte{0x00}:
                    // VX = VY
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
                    this->V->at(vx) = this->V->at(vy);
                    break;
                case std::byte{0x01}:
                    // VX = VX OR VY
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
                    this->V->at(vx) = this->V->at(vx) | this->V->at(vy);
                    break;
                case std::byte{0x02}:
                    // VX = VX AND VY
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
                    this->V->at(vx) = this->V->at(vx) & this->V->at(vy);
                    break;
                case std::byte{0x03}:
                    // VX = VX XOR VY
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
                    this->V->at(vx) = this->V->at(vx) ^ this->V->at(vy);
                    break;
                case std::byte{0x04}:
                    // VX = VX + VY
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
                    // old values for VX and VY
                    X = (uint8_t) this->V->at(vx);
                    Y = (uint8_t) this->V->at(vy);
                    result = X + Y;
                    this->V->at(vx) = std::byte{result};
                    // carry 
                    if (X > UINT8_MAX - Y)
                    {
                        // overflow
                        this->V->at(0xF) = std::byte{0x1};
                    }
                    else
                    {
                        this->V->at(0xF) = std::byte{0};
                    }
                    break;
                case std::byte{0x05}:
                    // VX = VX - VY
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
                    // old values for VX and VY
                    X = (uint8_t) this->V->at(vx);
                    Y = (uint8_t) this->V->at(vy);
                    result = X - Y;
                    this->V->at(vx) = std::byte{result};
                    //  carry
                    if (X > Y)
                    {
                        this->V->at(0xF) = std::byte{0x1};
                    }
                    else
                    {
                        // underflow
                        this->V->at(0xF) = std::byte{0};
                    }
                    break;
                case std::byte{0x06}:
                    // shift right
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
                    // old values for VX and VY
                    X = (uint8_t) this->V->at(vx);
                    Y = (uint8_t) this->V->at(vy);
                    result = X >> 1;
                    this->V->at(vx) = std::byte{result};
                    if ((X & 0x1) != 0)
                    {
                        this->V->at(0xF) = std::byte{0x1};
                    }
                    else
                    {
                        this->V->at(0xF) = std::byte{0};
                    }
                    break;
                case std::byte{0x07}:
                    // VX = VY - VX
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
                    // old values for VX and VY
                    X = (uint8_t) this->V->at(vx);
                    Y = (uint8_t) this->V->at(vy);
                    result = Y - X;
                    this->V->at(vx) = std::byte{result};
                    if (Y > X)
                    {
                        this->V->at(0xF) = std::byte{0x1};
                    }
                    else
                    {
                        // underflow
                        this->V->at(0xF) = std::byte{0};
                    }
                    break;
                case std::byte{0x0E}:
                    // shift left
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
                    // old values for VX and VY
                    X = (uint8_t) this->V->at(vx);
                    Y = (uint8_t) this->V->at(vy);
                    result = X << 1;
                    this->V->at(vx) = std::byte{result};
                    if ((X & 0x80) != 0x80)
                    {
                        this->V->at(0xF) = std::byte{0};
                    }
                    else
                    {
                        this->V->at(0xF) = std::byte{0x1};
                    }
                    break;
            }
            break;
        case std::byte{0x90}:
            // skip if VX != VY
            vx = (uint8_t)(n12 & SECOND_NIBBLE);
            vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
            if (this->V->at(vx) != this->V->at(vy))
            {
                this->PC += 2;
            }
            break;
        case std::byte{0xA0}:
            // set index
            index = (memory::mem_addr)(n12 & SECOND_NIBBLE) << 8 | (memory::mem_addr)n34;
            this->I = index;
            break;
        case std::byte{0xB0}:
            vx = (uint8_t)(n12 & SECOND_NIBBLE);
            #if ORIGINAL_B_JUMP
            vx = 0;
            #endif
            to = (memory::mem_addr)(n12 & SECOND_NIBBLE) << 8 | (memory::mem_addr)n34;
            to += (uint8_t) this->V->at(vx);
            this->PC = to;
            break;
        case std::byte{0xC0}:
            vx = (uint8_t)(n12 & SECOND_NIBBLE);
            result = (rand() % 0xFF) ^ (uint8_t)n34;
            this->V->at(vx) = std::byte{result};
            break;
        case std::byte{0xD0}:
            // Draw
            //X <- VX
            vx = (uint8_t)(n12 & SECOND_NIBBLE);
            X = (uint8_t)std::byte{this->V->at(vx)};
            //Y <- VY
            vy = (uint8_t)(n34 & FIRST_NIBBLE) >> 4;
            Y = (uint8_t)std::byte{this->V->at(vy)};
            //N
            N = (uint8_t)(n34 & SECOND_NIBBLE);
            sprite = new std::vector<std::byte>(N);
            // we read 
            for (memory::mem_addr offset = 0; offset < N; offset++)
            {
                sprite->at(offset) = this->ram->read(this->I + offset);
            }
            if (this->display->draw(X, Y, sprite) > 0)
            {
                this->V->at(0xF) = std::byte{1};
            }
            this->display->update();
            break;
        case std::byte{0xE0}:
            switch (n34)
            {
                case std::byte{0x9E}:
                    // skip instruction if key in VX is pressed
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    X = (uint8_t)(this->V->at(vx) & SECOND_NIBBLE);
                    if (this->keypad->is_pressed(X))
                    {
                        this->PC += 2;
                    }
                    break;
                case std::byte{0xA1}:
                    // skip instruction if key in VX is not pressed
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    X = (uint8_t)(this->V->at(vx) & SECOND_NIBBLE);
                    if (not this->keypad->is_pressed(X))
                    {
                        this->PC += 2;
                    }
                    break;
            }
            break;
        case std::byte{0xF0}:
            switch (n34)
            {
                case std::byte{0x07}:
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    this->V->at(vx) = std::byte{(uint8_t)this->delay_timer};
                    break;
                case std::byte{0x15}:
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    this->delay_timer = (uint8_t)this->V->at(vx);
                    break;
                case std::byte{0x18}:
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    this->sound_timer = (uint8_t)this->V->at(vx);
                    break;
                case std::byte{0x1E}:
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    this->I += (uint8_t) this->V->at(vx);
                    if (this->I >= 0x1000) {
                        this->V->at(0xF) = std::byte{1};
                    }
                    break;
                case std::byte{0x0A}:
                    // wait for key, store in VX
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    X = this->keypad->wait_for_key();
                    this->V->at(vx) = std::byte{X};
                    break;
                case std::byte{0x29}:
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    // the letter we want to print
                    to = FONT_START_AT +  5 * (uint8_t)(this->V->at(vx) & SECOND_NIBBLE);
                    this->I = to;
                    break;
                case std::byte{0x33}:
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    X = (uint8_t)this->V->at(vx);
                    // units
                    this->ram->write(this->I, std::byte{X/100});
                    // spdlog::info("{} takes {}", this->I, std::byte{X/100});
                    X %= 100;
                    // tens
                    this->ram->write(this->I + 1, std::byte{X/10});
                    // spdlog::info("{} takes {}", this->I+1, std::byte{X/10});
                    X %= 10;
                    // hundreds
                    this->ram->write(this->I + 2, std::byte{X%10});
                    // spdlog::info("{} takes {}", this->I+2, std::byte{X%10});
                    break;
                case std::byte{0x55}:
                    // store V0 to VX in memory starting at address I
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    for (uint8_t i = 0; i <= vx; i++)
                    {
                        this->ram->write(this->I + i, this->V->at(i));
                    }
                    #if ORIGINAL_STORE_MEM
                    this->I += vx + 1;
                    #endif
                    break;
                case std::byte{0x65}:
                    // load V0 to VX from memory starting at address I
                    vx = (uint8_t)(n12 & SECOND_NIBBLE);
                    for (uint8_t i = 0; i <= vx; i++)
                    {
                        this->V->at(i) = this->ram->read(this->I + i);
                    }
                    #if ORIGINAL_STORE_MEM
                    this->I += vx + 1;
                    #endif
                    break;
            }
            break;
    }
}
