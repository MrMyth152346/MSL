#pragma once

#include <algorithm>
#include <vector>

#include "Token.hh"

typedef enum
{
    MSL_SCOPE_IF,
    MSL_SCOPE_DO,
    MSL_SCOPE_VAR,
    MSL_SCOPE_CALL,
    MSL_SCOPE_NONE,
    MSL_SCOPE_WORD,
    MSL_SCOPE_STRING,
    MSL_SCOPE_NUMBER,
    MSL_SCOPE_FUNCTION,
    MSL_SCOPE_BRACKETS,
    MSL_SCOPE_OPERATION,
} MSL_SCOPE_TYPE;

size_t scopeDepth = 0;

class MSL_Scope
{
public:
    MSL_Tokens selves;
    MSL_Scope *parent;
    std::vector<MSL_Scope*> childs;

    size_t id;
    int type;

    void setParent(MSL_Scope *parent)
    {
        if (this->parent != nullptr)
        {
            auto it = std::find(this->childs.begin(), this->childs.end(), this);


            if (it != this->childs.end())
                this->parent->childs.erase(it);
        }

        this->parent = parent;
        this->parent->childs.push_back(this);
    }

    void setParent(std::vector<MSL_Scope*> *parent)
    {
        this->parent = nullptr;
        parent->push_back(this);
    }

    MSL_Scope()
    {
        this->id = 0;
        this->parent = 0ULL;
        this->type   = MSL_SCOPE_NONE;
    }
};

typedef MSL_Scope MSL_State;
typedef std::vector<MSL_Scope*> MSL_States;
typedef std::vector<MSL_Scope*> MSL_Scopes;

void MSL_DisplayScope(MSL_Scopes *scopes)
{
    size_t currentDepth = scopeDepth;

    for (size_t i = 0; i < scopes->size(); i++)
    {
        std::cout << "\n|" << scopes->at(i)->type << ", " << scopes->at(i)->id;
        scopeDepth++;
        currentDepth++;
        for (size_t y = scopeDepth - currentDepth; y < scopeDepth; y++)
            std::cout << " -";
        for (size_t x = 0; x < scopes->at(i)->selves.size(); x++)
            std::cout << scopes->at(i)->selves.at(x).value << " ";

        MSL_DisplayScope(&scopes->at(i)->childs);
        scopeDepth--;
        currentDepth--;
    }
}