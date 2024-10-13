#pragma once

#include <vector>
#include <unordered_map>

#include "Inform.hh"
#include "Scope.hh"

typedef enum
{
    MSL_VARIABLE_INT,
    MSL_VARIABLE_NULL,
    MSL_VARIABLE_DOUBLE,
    MSL_VARIABLE_STRING,
    MSL_VARIABLE_FUNCTION,
    MSL_VARIABLE_FUNCTION_CPP,
    MSL_VARIABLE_SCOPE,
} MSL_VARIABLE_TYPES;

typedef enum
{
    MSL_STACK_INT,
    MSL_STACK_NULL,
    MSL_STACK_DOUBLE,
    MSL_STACK_STRING,
    MSL_STACK_FUNCTION,
    MSL_STACK_FUNCTION_CPP,
    MSL_STACK_SCOPE,
} MSL_STACK_TYPES;

typedef class
{
public:
    int     type;
    void   *value;
} MSL_StackValue;

typedef class
{
public:
    int     type;
    size_t  id;
    void   *var;
} MSL_Variable;


typedef std::unordered_map<std::string, MSL_Variable*> MSL_Variables;
typedef std::vector<MSL_StackValue> MSL_Stack;

typedef class
{
private:
    MSL_Variables variables;
    MSL_Stack stack;

public:

    template <typename T>
    void stackPush(T value, int type)
    {
        MSL_StackValue stackValue;

        stackValue.value = value;
        stackValue.type  = type;

        this->stack.push_back(stackValue);
    }

    void stackPop()
    {
        this->stack.pop_back();
    }

    MSL_StackValue *stackGet()
    {
        MSL_StackValue *value;
        value = &stack[0];
        this->stack.erase(std::begin(this->stack));
        return value;
    }
    
    void clearStack()
    {
        this->stack.clear();
    }

    MSL_Stack* getStack()
    {
        return &stack;
    }

    template <typename T>
    void Push(size_t id, std::string name, T *value, int type)
    {
        MSL_Variable *variable = new MSL_Variable;
        variable->type = type;
        variable->id   = id;
        variable->var = value;

        this->variables[name] = variable;
    }

    void Push(size_t id, std::string name)
    {

        MSL_Variable *variable = new MSL_Variable;
        variable->type = MSL_VARIABLE_NULL;
        variable->id   = id;
        variable->var = nullptr;

        this->variables[name] = variable;
    }

    void Delete(MSL_Variable *variable)
    {

        switch (variable->type)
        {
        case MSL_VARIABLE_INT:
            delete (int*)variable->var;
            break;

        case MSL_VARIABLE_DOUBLE:
            delete (double*)variable->var;
            break;

        case MSL_VARIABLE_STRING:
            delete (std::string*)variable->var;
            break;

        case MSL_VARIABLE_FUNCTION:
            delete (MSL_Scope*)variable->var;
            break;

        case MSL_VARIABLE_FUNCTION_CPP:
            break;

        case MSL_VARIABLE_NULL:
            //std::cout << "NULL" << std::endl;
            break;
        }
    }

    template <typename T>
    void Change(std::string name, T *value, int type)
    {
        MSL_Variables::iterator index = this->variables.find(name);

        if (index != variables.end())
        {
            this->Delete(this->variables[name]);
            variables[name]->var = value;

            switch (type)
            {
            case MSL_VARIABLE_INT:
                variables[name]->type = MSL_VARIABLE_INT;
                break;
            case MSL_VARIABLE_DOUBLE:
                variables[name]->type = MSL_VARIABLE_DOUBLE;
                break;
            case MSL_VARIABLE_STRING:
                variables[name]->type = MSL_VARIABLE_INT;
                break;
            }
            
        }
        else
            MSL_Error("variable doesnt exist lamfo");
    }

    MSL_Variable* Get(std::string name)
    {
        MSL_Variables::iterator index = variables.find(name);
        
        if (index != variables.end())
        {
            return variables[name];
        }
        else
            MSL_Error("nono");

        MSL_Error("wtf");

        return nullptr;
    }

    void Display()
    {
        for (MSL_Variables::iterator it = variables.begin(); it != variables.end(); it++)
            std::cout << "Variable " << it->first << ": " << it->second->id << ", " << it->second->type << std::endl;
    }
} MSL_Memory;


int MSL_Interpret(MSL_Scopes* scopes, size_t *from, size_t count, MSL_Memory *memory);

char       lastOperator;
MSL_Scope *lastScope = nullptr;

double MSL_Sum(MSL_Token *token1, MSL_Token *tokenOp, MSL_Token *token2)
{
    double sum = 0;

    double num1 = std::stod(token1->value);
    double num2 = std::stod(token2->value);
    char   op   = tokenOp->value.c_str()[0];

    if (tokenOp->value.size() > 1)
        MSL_Error("fail op inter");

    switch (op)
    {
    case '*':
        sum = num1 * num2;
        break;
    case '/':
        sum = num1 / num2;
        break;
    case '+':
        sum = num1 + num2;
        break;
    case '-':
        sum = num1 - num2;
        break;
    
    default:
        MSL_Error("Imposibl op inter");
    }

    if (sum != (int)sum)
    {
        token1->value = std::to_string(sum);
        token2->value = std::to_string(sum);
    }
    else
    {
        token1->value = std::to_string((int)sum);
        token2->value = std::to_string((int)sum);
    }

    return sum;
}

double MSL_InterpretOpr(MSL_Scopes *scopes)
{
    double sum = 0;

    for (size_t pos = 0; pos < scopes->size(); pos++)
    {

        MSL_Scope *scope = scopes->at(pos);

        if (scope->parent->type == MSL_SCOPE_OPERATION && lastScope != nullptr && pos > 0)
            scopes->at(pos - 1)->selves = lastScope->selves;

        MSL_Token *token1  = nullptr;
        MSL_Token *tokenOp = nullptr;
        MSL_Token *token2  = nullptr;

        
        if (scope->parent->parent != nullptr && scope->parent->parent->type == MSL_SCOPE_OPERATION)
        {
            token1  = &(scope->parent->selves[0]);
            tokenOp = &(scope->parent->selves[1]);
            token2  = &(scope->selves[0]);
        }

        if (token1 != nullptr && tokenOp != nullptr && token2 != nullptr)
        {
            char op = tokenOp->value.c_str()[0];

            sum = MSL_Sum(token1, tokenOp, token2);

            lastOperator = op;
        }
        else if (scope->parent != nullptr && scope->parent->type == MSL_SCOPE_OPERATION && scope->parent->childs.size() == 1)
        {
            scope->parent->selves = scope->selves;
        }

        lastScope = scope;

        if (scope->childs.size() > 0)
        {
            double lastSum = sum;
            lastOperator = ' ';
            sum = MSL_InterpretOpr(&scope->childs);
            scope->childs.clear();
        }
        else
        {
            sum = std::stod(scope->selves[0].value);
        }
    }

    if (scopes->size() > 1)
    {
        for (size_t pos = 1; pos < scopes->size(); pos++)
        {
            MSL_Scope *scope = scopes->at(pos);

            MSL_Token *token1  = nullptr;
            MSL_Token *tokenOp = nullptr;
            MSL_Token *token2  = nullptr;

            token1  = &(scopes->at(pos - 1)->selves[0]);
            tokenOp = &(scopes->at(pos - 1)->selves[1]);
            token2  = &(scope->selves[0]);

            sum = MSL_Sum(token1, tokenOp, token2);
        }
    }


    return sum;
}

std::string MSL_GetVarName(MSL_Scope* scope)
{

    if (scope->selves.size() >= 2)
        return scope->selves[1].value;
    else
        return scope->selves[0].value;
}

void MSL_InterpretVar(MSL_Scope *scope, MSL_Memory *memory)
{
    // TODO: MAKE VARIABLES USE STACK

    size_t from = 0;

    memory->clearStack();

    MSL_Interpret(&scope->childs, &from, 1, memory);

    MSL_StackValue *stackValue = memory->stackGet();

    if (scope->selves[0].value == "var")
        memory->Push(scope->id, MSL_GetVarName(scope), stackValue->value, stackValue->type);
    else
        memory->Change(MSL_GetVarName(scope), stackValue->value, stackValue->type);

    memory->clearStack();
}

void MSL_InterpretFunc(MSL_Scope *scope, MSL_Memory *memory)
{
    std::string name = "";

    if (scope->selves.size() > 0)
        name = scope->selves[0].value;
    else
        name = "";

    memory->Push(scope->id, name, scope, MSL_VARIABLE_FUNCTION);
}

void MSL_InterpretCall(MSL_Scope scope, MSL_Memory *memory)
{
    std::string name = scope.selves[0].value;
    MSL_Variable *variable = memory->Get(name);

    memory->clearStack();

    if (variable->type == MSL_VARIABLE_FUNCTION)
    {
        MSL_Scope *funcScope = (MSL_Scope*)variable->var;

        size_t from = 0;
        MSL_Interpret(&scope.childs[0]->childs, &from, scope.childs[0]->childs.size(), memory);
        MSL_Interpret(&funcScope->childs[1]->childs, &from, funcScope->childs[1]->childs.size(), memory);
    }
    else if (variable->type == MSL_VARIABLE_FUNCTION_CPP)
    {
        size_t from = 0;
        MSL_Interpret(&scope.childs[0]->childs, &from, scope.childs[0]->childs.size(), memory);
        ((void (*)(MSL_Stack*, MSL_Memory*))variable->var)(memory->getStack(), memory);
    }
    else
        MSL_Error("Unexpected call on a variable.");

    memory->clearStack();
}

void MSL_InterpretWord(MSL_Scope scope, MSL_Memory *memory)
{
    std::string name = scope.selves[0].value;

    MSL_Variable *variable = memory->Get(name);

    memory->stackPush(variable->var, variable->type);
}

int MSL_Interpret(MSL_Scopes* scopes, size_t *from, size_t count, MSL_Memory *memory)
{
    size_t currentCount = 0;
    int    currentScope = MSL_SCOPE_NONE;

    for (*from; *from < scopes->size(); (*from)++)
    {
        if (currentCount >= count) 
            return currentScope;

        MSL_Scope *scope = scopes->at(*from);

        switch (scope->type)
        {
        case MSL_SCOPE_VAR:         
            MSL_InterpretVar(scope, memory);  
            break;
        case MSL_SCOPE_OPERATION:
        {
            lastOperator = ' ';
            lastScope = nullptr;
            double *sum = new double;
            *sum = MSL_InterpretOpr(&scope->childs);
            scope->selves.clear();
            scope->selves.push_back(MSL_Token(MSL_TOKEN_NUMBER, std::to_string(*sum), 0, 0));
            if ((int)*sum == *sum)
            {
                int *intSum = new int;
                *intSum = *sum;
                delete sum;
                memory->stackPush(intSum, MSL_STACK_INT);
            }
            else
                memory->stackPush(sum, MSL_STACK_DOUBLE);
            break;
        }
        case MSL_SCOPE_WORD:
            MSL_InterpretWord(*scope, memory);
            break;
        case MSL_SCOPE_CALL:
            MSL_InterpretCall(*scope, memory);
            memory->stackPush(scope, MSL_STACK_FUNCTION);
            break;
        case MSL_SCOPE_FUNCTION:
            MSL_InterpretFunc(scope, memory);
            break;
        case MSL_SCOPE_BRACKETS:
            MSL_Interpret(&scope->childs, 0, scope->childs.size(), memory);
            break;
        case MSL_SCOPE_DO:
            memory->stackPush(scope, MSL_STACK_SCOPE);
            break;
        case MSL_SCOPE_STRING:
            memory->stackPush(&scope->selves[0].value, MSL_STACK_STRING);
            break;
        default:
            MSL_Error("what");
        }


        currentScope = scope->type;
        currentCount++;
    }

    return currentScope;
}