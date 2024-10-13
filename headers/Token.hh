#pragma once

#include <string>

typedef enum
{
    MSL_TOKEN_STRING,
    MSL_TOKEN_OPERATOR,
    MSL_TOKEN_BRACKETS,
    MSL_TOKEN_KEYWORD,
    MSL_TOKEN_NUMBER,
    MSL_TOKEN_NONE,
    MSL_TOKEN_WS,
} MSL_TOKEN_TYPES;

class MSL_Token
{
public:
    int type;
    std::string value;

    size_t row;
    size_t col;

    MSL_Token(int type, std::string value, size_t row, size_t col)
    {
        this->type = type;
        this->value = value;
        this->row = row;
        this->col = col;
    }

    MSL_Token()
    {
        this->type = 0ULL;
        this->row   = 0ULL;
        this->col   = 0ULL;
        this->value = "";
    }
};

typedef std::vector<MSL_Token> MSL_Tokens;
typedef std::vector<MSL_Token*> MSL_TokensPtr;

void MSL_InsertToken(MSL_Token token, MSL_Tokens *tokens)
{
    if (token.type != MSL_TOKEN_NONE)
    {
        token.row -= token.value.size();
        tokens->push_back(token);
    }
}

void MSL_LayTokens(MSL_Tokens tokens, size_t col, size_t endCol)
{
    size_t lastRow = 0;
    size_t lastCol = col;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        MSL_Token token = tokens[i];

        if (token.col > col && token.col < endCol)
        {
            if (token.col - lastCol > 0) lastRow = 0;
            
            if (token.col > lastCol)
                for (int y = 0; y < token.col - lastCol; y++)
                {
                    std::cout << '\n';
                }
            if (token.row > lastRow)
                for (int x = 0; x < token.row - lastRow; x++)
                {
                    std::cout << ' ';
                }

            std::cout << token.value;

            lastRow = token.row + token.value.size();
            lastCol = token.col;
        }
    }

    std::cout << '\n';
}
