#pragma once

#include <SDL3_mixer/SDL_mixer.h>

#ifndef BEEP_SOUND_PATH
#define BEEP_SOUND_PATH "sounds/beep.wav"
#endif

namespace beep
{
    class Beeper
    {
        private:
            bool playing;
            Mix_Music* sample;
        public:
            Beeper();
            ~Beeper();
            void init();
            void start();
            void stop();
    };
}
