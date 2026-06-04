#include "ParetoTools.h"
#include <algorithm>
#include <cmath>
#include <limits>

std::vector<std::vector<int>> ParetoTools::AssignRanks(std::vector<Solution>& population) 
{
    const int size = static_cast<int>(population.size());

    std::vector<std::vector<int>> dominatedBy(size);
    std::vector<int> dominationCount(size, 0);
    std::vector<std::vector<int>> fronts(1);

    for (int i = 0; i < size; ++i) 
    {
        for (int j = 0; j < size; ++j) 
        {
            if (i == j) continue;

            if (Dominates(population[i].GetObjectives(), population[j].GetObjectives())) 
            {
                dominatedBy[i].push_back(j);
            } 
            else if (Dominates(population[j].GetObjectives(), population[i].GetObjectives())) 
            {
                dominationCount[i]++;
            }
        }

        if (dominationCount[i] == 0) 
        {
            population[i].SetRank(0);
            fronts[0].push_back(i);
        }
    }
    for (int rank = 0; rank < static_cast<int>(fronts.size()); ++rank) 
    {
        std::vector<int> nextFront;

        for (int i : fronts[rank]) 
        {
            for (int j : dominatedBy[i]) 
            {
                dominationCount[j]--;

                if (dominationCount[j] == 0) 
                {
                    population[j].SetRank(rank + 1);
                    nextFront.push_back(j);
                }
            }
        }

        if (!nextFront.empty()) 
        {
            fronts.push_back(std::move(nextFront));
        }
    }

    return fronts;
}

void ParetoTools::AssignCrowdingDistance(std::vector<Solution>& population, const std::vector<int>& front) 
    {
    if (front.empty()) return;

    for (int index : front) 
    {
        population[index].SetCrowdingDistance(0.0);
    }

    if (front.size() <= 2) 
    {
        for (int index : front) 
        {
            population[index].SetCrowdingDistance(std::numeric_limits<double>::infinity());
        }
        return;
    }
    AddCrowdingDistanceByObjective(population, front, true);
    AddCrowdingDistanceByObjective(population, front, false);
}

std::vector<Solution> ParetoTools::SelectBest(std::vector<Solution> candidates, int limit) 
{
    const auto fronts = AssignRanks(candidates);

    for (const auto& front : fronts) 
    {
        AssignCrowdingDistance(candidates, front);
    }

    std::sort(candidates.begin(), candidates.end(), [](const Solution& left, const Solution& right) 
    {
        if (left.GetRank() != right.GetRank()) 
        {
            return left.GetRank() < right.GetRank();
        }

        return left.GetCrowdingDistance() > right.GetCrowdingDistance();
    });

    if (static_cast<int>(candidates.size()) > limit) 
    {
        candidates.resize(limit);
    }

    return candidates;
}

std::vector<Solution> ParetoTools::NonDominated(const std::vector<Solution>& solutions) 
{
    std::vector<Solution> result;

    for (size_t i = 0; i < solutions.size(); ++i) 
    {
        bool dominatedSolution = false;

        for (size_t j = 0; j < solutions.size(); ++j) 
        {
            if (i != j && Dominates(solutions[j].GetObjectives(), solutions[i].GetObjectives())) 
            {
                dominatedSolution = true;
                break;
            }
        }

        if (!dominatedSolution) 
        {
            result.push_back(solutions[i]);
        }
    }

    return result;
}

double ParetoTools::ObjectiveValue(const Solution& solution, bool serviceCostObjective) 
{
    return serviceCostObjective
        ? solution.GetObjectives().serviceCost
        : solution.GetObjectives().delayCost;
}

void ParetoTools::AddCrowdingDistanceByObjective(
    std::vector<Solution>& population,
    const std::vector<int>& front,
    bool serviceCostObjective) 
    {
    std::vector<int> sorted = front;

    std::sort(sorted.begin(), sorted.end(), [&](int left, int right) 
    {
        return ObjectiveValue(population[left], serviceCostObjective) <
               ObjectiveValue(population[right], serviceCostObjective);
    });

    population[sorted.front()].SetCrowdingDistance(std::numeric_limits<double>::infinity());
    population[sorted.back()].SetCrowdingDistance(std::numeric_limits<double>::infinity());

    const double minValue = ObjectiveValue(population[sorted.front()], serviceCostObjective);
    const double maxValue = ObjectiveValue(population[sorted.back()], serviceCostObjective);

    if (std::abs(maxValue - minValue) < 1e-12) return;

    for (size_t i = 1; i + 1 < sorted.size(); ++i) 
    {
        const double previous = ObjectiveValue(population[sorted[i - 1]], serviceCostObjective);
        const double next = ObjectiveValue(population[sorted[i + 1]], serviceCostObjective);
        population[sorted[i]].AddCrowdingDistance((next - previous) / (maxValue - minValue));
    }
}