#include "Solution.h"

Solution::Solution(std::vector<int> chromosome) : m_chromosome(std::move(chromosome)) {}

std::vector<int>& Solution::GetChromosome() 
{ 
    return m_chromosome; 
}

const std::vector<int>& Solution::GetChromosome() const 
{ 
    return m_chromosome; 
}

const Objectives& Solution::GetObjectives() const 
{ 
    return m_objectives; 
}

void Solution::SetObjectives(const Objectives& objectives) 
{ 
    m_objectives = objectives; 
}

int Solution::GetRank() const 
{ 
    return m_rank; 
}

void Solution::SetRank(int rank) 
{ 
    m_rank = rank; 
}

double Solution::GetCrowdingDistance() const 
{  
    return m_crowdingDistance; 
}

void Solution::SetCrowdingDistance(double distance) 
{ 
    m_crowdingDistance = distance; 
}

void Solution::AddCrowdingDistance(double distance) 
{ 
    m_crowdingDistance += distance; 
}
