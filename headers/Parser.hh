#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Scope.hh"
#include "Inform.hh"

MSL_Scopes *MSL_ParseUntil(MSL_Tokens *tokens, size_t *from, std::string value);
MSL_Scopes *MSL_Parse(MSL_Tokens *tokens, size_t *from, size_t count);

MSL_Scope **currentScope = new MSL_Scope*;
MSL_Scopes *scopes       = new MSL_Scopes;
MSL_Scope   spaceScope   = MSL_Scope{};

size_t scopeId = 0;

void MSL_ParserError(std::string what, size_t row, size_t col, MSL_Tokens tokens)
{
    MSL_LayTokens(tokens, col, col);

    MSL_Error(what + ". " + std::to_string(row) + " : " + std::to_string(col));
}

void MSL_ParseOperation(MSL_Scopes *scopes, MSL_Scope **currentScope, MSL_Tokens *tokens, size_t *parserIndex)
{    
    MSL_Scope *scope     = new MSL_Scope;
    MSL_Scope *endScope  = new MSL_Scope;
    MSL_Scope *lastScope = scope;

    scope->parent = nullptr;
    scope->type = MSL_SCOPE_OPERATION;
    scope->id = scopeId;

    int lastType = MSL_TOKEN_NONE;
    std::string lastOperator = "";
    MSL_Tokens *lastSelves;

    for (size_t i = *parserIndex; i < tokens->size(); i++)
    {
        MSL_Token currentToken = tokens->at(i);

        if (currentToken.type == MSL_TOKEN_OPERATOR && lastType != MSL_TOKEN_OPERATOR)
        {
            MSL_Scope *opScope = new MSL_Scope;

            opScope->selves.push_back(tokens->at(i - 1));
            opScope->selves.push_back(currentToken);
            opScope->type = MSL_SCOPE_OPERATION;

            if (currentToken.value != "*" && currentToken.value != "/")
            {
                if (lastOperator == "*" || lastOperator == "/")
                {
                    opScope->setParent(lastScope);
                    if (currentToken.value == "-")
                        lastScope = opScope;
                    else
                        lastScope = scope;
                }
                else
                {
                    if (lastScope != nullptr && lastOperator == "-")
                        opScope->setParent(lastScope);
                    else
                        opScope->setParent(endScope);

                    lastScope = scope;
                }
            }
            else
            {
                opScope->setParent(lastScope);
                lastScope = opScope;
            }
            
            lastType = MSL_TOKEN_OPERATOR;
            lastOperator = currentToken.value;
            lastSelves = &opScope->selves;
        }
        else if (currentToken.type == MSL_TOKEN_NUMBER && lastType != MSL_TOKEN_NUMBER && i < tokens->size() - 1 && tokens->at(i+1).type == MSL_TOKEN_OPERATOR)
        {
            lastType = MSL_TOKEN_NUMBER;
        }
        else if (currentToken.type == MSL_TOKEN_OPERATOR)
        {
            MSL_ParserError("Unexpected operator", currentToken.row, currentToken.col, *tokens);
        }
        else if (currentToken.type == MSL_TOKEN_NUMBER)
        {
            MSL_Scope *opScope = new MSL_Scope;

            opScope->selves.push_back(currentToken);
            opScope->type = MSL_SCOPE_OPERATION;

            if (lastOperator == "*" || lastOperator == "/")
            {
                if (endScope->childs.size() > 0)
                {
                    opScope->selves.push_back(endScope->childs[endScope->childs.size() - 1]->selves.at(1));
                    endScope->childs[endScope->childs.size() - 1]->selves.pop_back();
                }

                opScope->setParent(lastScope);
                lastScope = opScope;
            }
            else
            {
                opScope->setParent(endScope);
            }

            break;
        }
        else
        {
            MSL_ParserError("Unexpected operator", currentToken.row, currentToken.col, *tokens);
            break;
        }

        (*parserIndex)++;
    }


        if ((*currentScope)->type != MSL_SCOPE_NONE)
            scope->setParent(*currentScope);
        else
            scope->setParent(scopes);
            
        for (MSL_Scope *scopeChild : endScope->childs)
            scopeChild->setParent(scope);

    /*

    for (MSL_Scope *escope : endScope->childs)
        if ((*currentScope)->type != MSL_SCOPE_NONE)
            escope->setParent(scopes);
        else
            escope->setParent(*currentScope);
    */
}

void MSL_ParseDo(MSL_Scopes *scopes, MSL_Scope **currentScope, MSL_Tokens *tokens, size_t *parserIndex)
{
    scopeId++;

    MSL_Scope *scope = new MSL_Scope;
    scope->type = MSL_SCOPE_DO;
    scope->id = scopeId;
    scope->selves.push_back(tokens->at(*parserIndex));

    if ((*currentScope)->type != MSL_SCOPE_NONE)
        scope->setParent(*currentScope);
    else
        scope->setParent(scopes);

    (*currentScope) = scope;

    (*parserIndex)++;
    
    MSL_ParseUntil(tokens, parserIndex, "end");
}

void MSL_ParseEnd(MSL_Scopes *scopes, MSL_Scope **currentScope, MSL_Tokens *tokens, size_t *parserIndex)
{
    (*currentScope) = (*currentScope)->parent;
    scopeId--;
}

void MSL_ParseVar(MSL_Scopes *scopes, MSL_Scope **currentScope, MSL_Tokens *tokens, size_t *parserIndex)
{
    MSL_Scope *scope = new MSL_Scope;
    scope->id = scopeId;

    if (*currentScope != nullptr && (*currentScope)->type != MSL_SCOPE_NONE)
        scope->setParent(*currentScope);
    else
        scope->setParent(scopes);

    bool declaration = false;
    bool gotName = false;
    bool gotValue = false;

    if (tokens->at(*parserIndex).value == "var")
    {
        scope->selves.push_back(tokens->at(*parserIndex));
        declaration = true;
    }

    scope->type = MSL_SCOPE_VAR;


    for (size_t x = *parserIndex + 1; x < tokens->size(); x++)
    {

        if (tokens->at(x).type == MSL_TOKEN_KEYWORD && gotName == false && x - *parserIndex == 1)
        {   
            gotName = true;
            scope->selves.push_back(tokens->at(x));
            (*parserIndex)++;
        }
        else if (tokens->at(x).value == "=" && gotName == true && x - *parserIndex == 1)
        {
            gotValue = true;

            *currentScope = scope;
            
            (*parserIndex)+=2;

            MSL_Parse(tokens, parserIndex, 1);
            
            (*parserIndex)--;


            if ((*currentScope)->parent == nullptr)
                *currentScope = &spaceScope;
            else
                (*currentScope) = (*currentScope)->parent;
            break;
        }
        else if (gotName == true && gotValue == false)
            break;
        else
            MSL_ParserError("Unexpected word while variable declaration " + tokens->at(x).value, tokens->at(x).row, tokens->at(x).col, *tokens);
    }
}

void MSL_ParseBrackets(MSL_Scopes *scopes, MSL_Scope **currentScope, MSL_Tokens *tokens, size_t *parserIndex)
{
    if (tokens->at(*parserIndex).value == "(")
    {
        MSL_Scope *scope = new MSL_Scope;
        scope->type = MSL_SCOPE_BRACKETS;
        scope->id = scopeId;

        scope->selves.push_back(tokens->at(*parserIndex));

        if (*currentScope != nullptr && (*currentScope)->type != MSL_SCOPE_NONE)
            scope->setParent(*currentScope);
        else
            scope->setParent(scopes);

        (*currentScope) = scope;

        (*parserIndex)++;


        MSL_ParseUntil(tokens, parserIndex, ")");
    }
    else
    {
        (*currentScope) = (*currentScope)->parent;
    }
}

void MSL_ParseFunc(MSL_Scopes *scopes, MSL_Scope **currentScope, MSL_Tokens *tokens, size_t *parserIndex)
{

    MSL_Scope *scope = new MSL_Scope;
    scope->id = scopeId;

    scopeId++;

    if (*currentScope != nullptr && (*currentScope)->type != MSL_SCOPE_NONE)
        scope->setParent(*currentScope);
    else
        scope->setParent(scopes);

    scope->type = MSL_SCOPE_FUNCTION;

    bool gotName = false;
    bool gotValue = false;

    for (size_t x = *parserIndex + 1; x < tokens->size(); x++)
    {
        if (tokens->at(x).type == MSL_TOKEN_KEYWORD && gotName == false && x - *parserIndex == 1)
        {
            gotName = true;
            scope->selves.push_back(tokens->at(x));
            (*parserIndex)++;
        }
        else if (tokens->at(x).type == MSL_TOKEN_BRACKETS && x - *parserIndex == 1)
        {
            gotValue = true;

            *currentScope = scope;
            (*parserIndex)++;

            MSL_Parse(tokens, parserIndex, 2);

            (*parserIndex)--;

            if ((*currentScope)->parent == nullptr)
                *currentScope = &spaceScope;
            else
                (*currentScope) = (*currentScope)->parent;
            
            break;
        }
        else if (gotName == true && gotValue == false)
            break;
        else
            MSL_ParserError("Unexpected word while function declaration " + tokens->at(x).value, tokens->at(x).row, tokens->at(x).col, *tokens);
    }

    scopeId--;
}

void MSL_ParseWord(MSL_Scopes *scopes, MSL_Scope **currentScope, MSL_Tokens *tokens, size_t *parserIndex)
{
    for (size_t x = *parserIndex + 1; x < tokens->size(); x++)
    {
        if (tokens->at(x).value == "=" && x - *parserIndex == 1)
        {
            (*parserIndex)--;
            MSL_ParseVar(scopes, currentScope, tokens, parserIndex);
            break;
        }
        else if (tokens->at(x).value == "(" && x - *parserIndex == 1)
        {
            MSL_Scope *scope = new MSL_Scope;
            scope->id   = scopeId;
            scope->type = MSL_SCOPE_CALL;

            if (*currentScope != nullptr && (*currentScope)->type != MSL_SCOPE_NONE)
                scope->setParent(*currentScope);
            else
                scope->setParent(scopes);

            scope->selves.push_back(tokens->at(*parserIndex));
            (*parserIndex)++;

            (*currentScope) = scope;

            MSL_ParseBrackets(scopes, currentScope, tokens, parserIndex);

            (*currentScope) = scope->parent;

            break;
        }
        else
        {
            MSL_Scope *scope = new MSL_Scope;
            scope->id   = scopeId;
            scope->type = MSL_SCOPE_WORD;

            if (*currentScope != nullptr && (*currentScope)->type != MSL_SCOPE_NONE)
                scope->setParent(*currentScope);
            else
                scope->setParent(scopes);

            scope->selves.push_back(tokens->at(*parserIndex));

            break;
        }
    }
}

void MSL_ParseAny(MSL_Scopes *scopes, MSL_Scope **currentScope, MSL_Token *token, int type, size_t *from)
{
    MSL_Scope *scope = new MSL_Scope;

    scope->type = type;
    scope->selves.push_back(*token);
    scope->id = scopeId;

    if (*currentScope != nullptr && (*currentScope)->type != MSL_SCOPE_NONE)
        scope->setParent(*currentScope);
    else
        scope->setParent(scopes);
}

void MSL_ParseNext(MSL_Tokens *tokens, MSL_Token token, size_t *from)
{
    if      (token.value == "var")             MSL_ParseVar(scopes, currentScope, tokens, from);
    else if (token.value == "do")              MSL_ParseDo(scopes, currentScope, tokens, from);
    else if (token.value == "end")             MSL_ParseEnd(scopes, currentScope, tokens, from);
    else if (token.value == "func")            MSL_ParseFunc(scopes, currentScope, tokens, from);
    else if (token.type == MSL_TOKEN_BRACKETS) MSL_ParseBrackets(scopes, currentScope, tokens, from);
    else if (token.type == MSL_TOKEN_KEYWORD)  MSL_ParseWord(scopes, currentScope, tokens, from);
    else if (token.type == MSL_TOKEN_NUMBER)   MSL_ParseOperation(scopes, currentScope, tokens, from);
    else if (token.type == MSL_TOKEN_STRING)   MSL_ParseAny(scopes, currentScope, &token, MSL_SCOPE_STRING, from);

    else MSL_ParserError("Parser " + token.value + ", " + std::to_string(token.type), token.row, token.col, *tokens);
}

MSL_Scopes *MSL_ParseUntil(MSL_Tokens *tokens, size_t *from, std::string value)
{

    for (*from; (*from) < tokens->size(); (*from)++)
    {
        MSL_Token token = tokens->at(*from);

        MSL_ParseNext(tokens, token, from);

        if (*from == tokens->size())
            MSL_ParserError("Expected " + value, token.row, token.col, *tokens);

        if (token.value == value)
        {
            
            break;
        }
    }

    return scopes;
}

MSL_Scopes *MSL_Parse(MSL_Tokens *tokens, size_t *from, size_t count)
{
    size_t currentScopeCount = 0;

    for (*from; (*from) < tokens->size(); (*from)++)
    {
        if (currentScopeCount >= count)
            break;

        MSL_Token token = tokens->at(*from);

        MSL_ParseNext(tokens, token, from);

        currentScopeCount++;
    }

    return scopes;
}

void MSL_ParserInit()
{
    spaceScope.parent = nullptr;
    *currentScope     = &spaceScope;
}