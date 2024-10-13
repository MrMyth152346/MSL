#pragma once

#include <iostream>

#include "MSL.hh"

void print_(MSL_Stack *stack, MSL_Memory *memory)
{   
    for (size_t i = 0; i < stack->size(); i++)
    {
        switch (stack->at(i).type)
        {
        case MSL_STACK_STRING:
            std::cout.write((*(std::string*)stack->at(i).value).c_str(), (*(std::string*)stack->at(i).value).size());
            break;
        case MSL_STACK_INT:
            std::cout << *(int*)stack->at(i).value;
            break;
        case MSL_STACK_DOUBLE:
            std::cout << *(double*)stack->at(i).value;
            break;
        }
    }

    std::cout.put('\n');
}

void for_(MSL_Stack *stack, MSL_Memory *memory)
{

    int *from = 0;
    int *to   = 0;
    MSL_State *state;

    if (stack->at(0).type != MSL_VARIABLE_INT || stack->at(1).type != MSL_VARIABLE_INT)
    {
        MSL_Error("Unexpected parameters: " + std::to_string(stack->size()));
    }

    from  = (int*)stack->at(0).value;
    to    = (int*)stack->at(1).value;
    state = (MSL_State*)stack->at(2).value;

    memory->clearStack();

    for (*from; *from < *to; (*from)++)
    {
        switch (state->type)
        {
        case MSL_SCOPE_DO:
        {
            size_t from_ = 0;
            MSL_Interpret(&state->childs, &from_, state->childs.size(), memory);
            break;
        }
        case MSL_SCOPE_FUNCTION:
            MSL_InterpretCall(*state, memory);
            break;

        default:
            MSL_Error("Unexpected argument.");
            break;
        }
    }
}