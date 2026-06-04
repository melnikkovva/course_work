#pragma once
#include <limits>
#include <vector>
#include "../Objectives/Objectives.h"

class Solution 
{
public:
    Solution() = default;
    explicit Solution(std::vector<int> chromosome);

    const std::vector<int>& GetChromosome() const;
    std::vector<int>& GetChromosome();
    const Objectives& GetObjectives() const;
    void SetObjectives(const Objectives& objectives);
    int GetRank() const;
    void SetRank(int rank);
    double GetCrowdingDistance() const;
    void SetCrowdingDistance(double distance);
    void AddCrowdingDistance(double distance);
private:
    std::vector<int> m_chromosome;
    Objectives m_objectives;
    int m_rank = std::numeric_limits<int>::max();
    double m_crowdingDistance = 0.0;
};