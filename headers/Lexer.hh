#pragma once

#include <iostream>
#include <vector>

#include "Token.hh"
#include "Inform.hh"

void MSL_LexerError(std::string what, size_t row, size_t col, MSL_Tokens tokens)
{
    MSL_LayTokens(tokens, col, col);

    MSL_Error(what + ". " + std::to_string(row) + " : " + std::to_string(col));
}

MSL_Tokens *MSL_Tokenize(std::string rawCode)
{
    MSL_Tokens *tokens = new MSL_Tokens;
    size_t row         = 0;
    size_t col         = 0;
    std::string value  = "";
    int type           = MSL_TOKEN_NONE;

    rawCode += '\0';

    for (size_t i = 0; i < rawCode.size(); i++)
    {
        char currentChar = rawCode[i];

        if (type == MSL_TOKEN_WS)
            type = MSL_TOKEN_NONE;
 
        switch (currentChar)
        {
            case '"':
                MSL_InsertToken(MSL_Token{type, value, row, col}, tokens);
                value = "";
                type  = MSL_TOKEN_STRING;
                i++;
                for (i; i < rawCode.size(); i++)
                {
                    char character = rawCode[i];

                    if (character == '"')
                    {

                        MSL_InsertToken(MSL_Token{type, value, row, col}, tokens);
                        break;
                    }
                    else
                        value += character;

                    if (i + 1 >= rawCode.size() - 1)
                        MSL_LexerError((std::string)"Missing " + '"', row, col, *tokens);
                }
                value = "";
                type  = MSL_TOKEN_NONE;

                break;

            case ' ':
                MSL_InsertToken(MSL_Token{type, value, row, col}, tokens);
                value = "";
                type  = MSL_TOKEN_WS;
                break;

            case '\n':
                MSL_InsertToken(MSL_Token{type, value, row, col}, tokens);
                value = "";
                type  = MSL_TOKEN_WS;
                row   = -1;
                col  += 1;
                break;

            case '\0':
                MSL_InsertToken(MSL_Token{type, value, row, col}, tokens);
                break;

            case '_':
            case 'A' ... 'Z':
            case 'a' ... 'z':
                if (type == MSL_TOKEN_NONE)
                {
                    type = MSL_TOKEN_KEYWORD;
                    value += currentChar;
                }
                else if (type == MSL_TOKEN_KEYWORD)
                {
                    value += currentChar;
                }
                else
                {
                    MSL_InsertToken(MSL_Token{type, value, row, col}, tokens);
                    value = currentChar;
                    type  = MSL_TOKEN_KEYWORD;
                }
                break;

            case '.':
            case '0' ... '9':
                if (currentChar != '.' && type == MSL_TOKEN_KEYWORD)
                    value += currentChar;
                else if (type == MSL_TOKEN_NONE)
                {
                    type = MSL_TOKEN_NUMBER;
                    value += currentChar;
                }
                else if (type == MSL_TOKEN_NUMBER)
                    value += currentChar;
                else
                {
                    MSL_InsertToken(MSL_Token{type, value, row, col}, tokens);
                    value = currentChar;
                    type  = MSL_TOKEN_NUMBER;
                }
                break;
                
            case '(' ... ')':
                MSL_InsertToken(MSL_Token{type, value, row, col}, tokens);

                value = currentChar;
                type  = MSL_TOKEN_BRACKETS;

                MSL_InsertToken(MSL_Token{type, value, row + 1, col}, tokens);

                value = "";
                type  = MSL_TOKEN_NONE;
                break;
            
            case '-':
            case '*':
            case '+':
            case '/':
            case '<' ... '>':
                if (type == MSL_TOKEN_NONE)
                {
                    type = MSL_TOKEN_OPERATOR;
                    value = currentChar;
                }
                else if (type == MSL_TOKEN_OPERATOR)
                    value += currentChar;
                else
                {
                    MSL_InsertToken(MSL_Token{type, value, row, col}, tokens);

                    value = currentChar;
                    type  = MSL_TOKEN_OPERATOR;
                }
                break;

            default:
                MSL_LexerError((std::string)"Unknown character " + "'" + currentChar + "'", row, col, *tokens);
        }
            

        row += 1;

        //std::cout << value << ", " << currentChar << " : " << "*" << i << "* " << row << ", " << col << std::endl;
    }

    return tokens;
}