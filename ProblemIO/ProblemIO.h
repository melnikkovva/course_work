#pragma once

#include <string>
#include "../Problem/Problem.h"
#include "../Solution/Solution.h"

class ProblemIO
{
public:

    static Problem LoadFromFile(
        const std::string& filename);

    static void SaveSolution(
        const Problem& problem,
        const Solution& solution,
        const std::string& filename);
};