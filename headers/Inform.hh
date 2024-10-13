#pragma once

#include <iostream>
#include <string>

void MSL_Error(std::string what)
{
    std::cerr << "MSL ERROR: " << what;
    exit(0);
}