#include <random>

double Random01(std::mt19937 m_rng) 
{
    return std::uniform_real_distribution<double>(0.0, 1.0)(m_rng);
}

int RandomIndex(size_t size, std::mt19937 m_rng) 
{
    return std::uniform_int_distribution<int>(0, static_cast<int>(size) - 1)(m_rng);
}
