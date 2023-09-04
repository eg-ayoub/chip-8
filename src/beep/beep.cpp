#include <beep/beep.hpp>

#include <SDL3/SDL.h>

#include <spdlog/spdlog.h>

beep::Beeper::Beeper()
{
    // nothing to do here
}

beep::Beeper::~Beeper()
{
    Mix_FreeMusic(sample);
    sample = NULL;
}

void beep::Beeper::init()
{
    this->playing = false;

    SDL_AudioSpec spec;
    spec.freq = 48000;
    spec.format = MIX_DEFAULT_FORMAT;
    spec.channels = 2;

    spdlog::info("initlizing SDL Mixer");
    if (Mix_OpenAudio(0, &spec) < 0)
    {
        throw std::runtime_error(std::format("unable to init SDL mixer: {}", Mix_GetError()));
    }
    
    spdlog::info("loading beep sound");
    sample = Mix_LoadMUS(BEEP_SOUND_PATH);
    if (sample == NULL)
    {
        throw std::runtime_error(std::format("unable to load beep sound: {}", Mix_GetError()));
    }
}

void beep::Beeper::start()
{
    if (Mix_PlayingMusic() == 0)
    {
        Mix_PlayMusic(this->sample, -1);
    }
    else
    {
        if(Mix_PausedMusic() == 1)
        {
            Mix_ResumeMusic();
        }
    }
}

void beep::Beeper::stop()
{
    if (Mix_PlayingMusic() == 1 & Mix_PausedMusic() == 0)
    {
        Mix_PauseMusic();
    }
}
