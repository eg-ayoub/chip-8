#pragma once

#include <vector>

#include <memory/memory.hpp>

#ifndef STACK_SIZE
#define STACK_SIZE 16
#endif

namespace stack
{
    class Stack
    {
    private:
        std::vector<memory::mem_addr> *s;
        int top;

    public:
        Stack();
        ~Stack();
        void init();
        void push(memory::mem_addr data);
        memory::mem_addr pop();
    };
}