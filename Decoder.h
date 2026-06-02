#pragma once
#include <vector>
#include "Problem.h"
#include "Route.h"
#include "Solution.h"

class Decoder 
{
public:
    explicit Decoder(const Problem& problem);
    std::vector<Route> Decode(const Solution& solution) const;
    Solution Encode(const std::vector<Route>& routes) const;
private:
    const Problem& m_problem;
};