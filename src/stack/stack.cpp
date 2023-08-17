#include <stack/stack.hpp>
#include <spdlog/spdlog.h>

stack::Stack::Stack()
{
    // nothing to do
}

stack::Stack::~Stack()
{
    this->s->clear();
    delete this->s;
}

void stack::Stack::init()
{
    spdlog::info("allocating stack");
    this->s = new std::vector<memory::mem_addr>(STACK_SIZE);

    top = -1;
}

void stack::Stack::push(memory::mem_addr data)
{
    if (top + 1 > STACK_SIZE)
    {
        throw std::runtime_error("stack overflow");
    }
    this->s->at(++top) = data;
}

memory::mem_addr stack::Stack::pop()
{
    if (top < 0)
    {
        throw std::runtime_error("empty stack");
    }
    return this->s->at(top--);
}