#pragma once

#include <vector>
#include "Solution.h"

class ParetoTools 
{
public:
    static std::vector<std::vector<int>> AssignRanks(std::vector<Solution>& population);
    static void AssignCrowdingDistance(std::vector<Solution>& population, const std::vector<int>& front);
    static std::vector<Solution> SelectBest(std::vector<Solution> candidates, int limit);
    static std::vector<Solution> NonDominated(const std::vector<Solution>& solutions);

private:
    static double ObjectiveValue(const Solution& solution, bool serviceCostObjective);
    static void AddCrowdingDistanceByObjective(
        std::vector<Solution>& population,
        const std::vector<int>& front,
        bool serviceCostObjective
    );
};