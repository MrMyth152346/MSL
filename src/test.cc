#include <iostream>
#include <math.h>


#include "../headers/Interpreter.hh"
#include "../headers/Parser.hh"
#include "../headers/Scope.hh"
#include "../headers/Lexer.hh"
#include "../headers/Token.hh"
#include "../headers/Functions.hh"

int main()
{
    std::string forCode = R"(
        var from = 0
        var to   = 1000

        for (from to do
            print("Hello, World! <" from ">")
        end)
    )";

    std::string functionCode = R"(
        func hello() do
            print("Hello, World!")
        end

        hello()
    )";

    std::string scopeCode = R"(
        var from = 0
        var to   = 1000

        var scope = do
            print("Hello, World! <" from ">")
        end

        for (from to scope)
    )";

    std::string binaryOperationCode = R"(
        var operation1 = 5 * 5 + 2
        var operation2 = 2 - 5 * 5
        var operation3 = 5 / 2 - 5 * 2

        print("Operation 1: " operation1)
        print("Operation 2: " operation2)
        print("Operation 3: " operation3)
    )";

    std::string returnCode = R"(
        func hello() do
            ret "hello"
        end

        print(hello())
    )";

    MSL_Tokens *tokens = MSL_Tokenize(binaryOperationCode);
    
    //MSL_LayTokens(*tokens, 0, 15);

    size_t from = 0;
    MSL_Memory memory = MSL_Memory();

    memory.Push(0, "print", (void*)&print_, MSL_VARIABLE_FUNCTION_CPP);
    memory.Push(0, "for",   (void*)&for_,   MSL_VARIABLE_FUNCTION_CPP);

    MSL_ParserInit();
    MSL_Scopes *scopesPtr = MSL_Parse(tokens, &from, pow(2, sizeof(size_t)));
    //MSL_DisplayScope(scopesPtr);
    
    //std::cout << '\n' << "PARSER END" << std::endl;

    from = 0;
    MSL_Interpret(scopes, &from, pow(2, sizeof(size_t)), &memory);

    //std::cout << "INTERPRETED" << std::endl;

    //memory.Display();

    return 0;
}
