#pragma once
#include <string>
#include "../Problem/Problem.h"

class ProblemLoader
{
public:
    static Problem LoadFromFile(const std::string& filename);
};