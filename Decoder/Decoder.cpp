#include "Decoder.h"

#include <optional>

Decoder::Decoder(const Problem& problem) : m_problem(problem) {}

std::vector<Route> Decoder::Decode(const Solution& solution) const 
{
    std::vector<Route> routes;
    std::optional<int> currentCaregiver;
    for (int gene : solution.GetChromosome()) 
    {
        if (gene > m_problem.customerCount()) 
        {
            currentCaregiver = gene;
            routes.emplace_back(gene);
        } 
        else if (currentCaregiver.has_value() && !routes.empty()) 
        {
            routes.back().GetCustomers().push_back(gene);
        }
    }
    return routes;
}

Solution Decoder::Encode(const std::vector<Route>& routes) const 
{
    std::vector<int> chromosome;
    
    for (const Route& route : routes) 
    {
        chromosome.push_back(route.GetCaregiverId());
        
        for (int customerId : route.GetCustomers()) 
        {
            chromosome.push_back(customerId);
        }
    }
    
    return Solution(chromosome);
}