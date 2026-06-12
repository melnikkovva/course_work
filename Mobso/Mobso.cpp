#include "MOBSO.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <numeric>
#include <unordered_set>
#include "../MathUtils.cpp"
#include "../ParetoTools/ParetoTools.h"

Mobso::Mobso(const Problem& problem, Params params)
    : m_problem(problem), m_params(params), m_rng(params.seed), 
      m_evaluator(problem), m_decoder(problem) {}

      std::vector<Solution> Mobso::run() 
    {
    std::vector<Solution> population = InitializePopulation();
    Evaluate(population);
    std::vector<Solution> archive = ParetoTools::NonDominated(population);
    archive = ParetoTools::Unique(archive);

    int evaluations = static_cast<int>(population.size());

    while (evaluations < m_params.maxEvaluations)
    {
        std::vector<Solution> children = MakeChildren(population);
        Evaluate(children);
        evaluations += static_cast<int>(children.size());
        population.insert(population.end(), children.begin(), children.end());
        population = ParetoTools::SelectBest(population, m_params.populationSize);
        archive.insert(archive.end(), population.begin(), population.end());
        RemoveInfeasible(archive);

        archive = ParetoTools::NonDominated(archive);
        archive = ParetoTools::Unique(archive);
    }

    return archive;
}

std::vector<Solution> Mobso::InitializePopulation()
{
    std::vector<Solution> population;
    population.reserve(m_params.populationSize);

    for (int i = 0; i < m_params.populationSize; ++i)
    {
        population.push_back(MakeFeasibleInitialSolution());
    }

    return population;
}

const Route& FindRouteByCaregiver(const std::vector<Route>& routes, int caregiverId)
{
    auto it = std::find_if(routes.begin(), routes.end(), [caregiverId](const Route& route)
    {
        return route.GetCaregiverId() == caregiverId;
    });

    if (it == routes.end())
    {
        throw std::logic_error("Route for caregiver was not found");
    }

    return *it;
}

std::vector<int> CenterIds(const Problem& problem)
{
    std::vector<int> result;

    for (const auto& center : problem.centers())
    {
        result.push_back(center.GetId());
    }

    return result;
}

Solution Mobso::MakeFeasibleInitialSolution()
{
    std::vector<Route> routes;

    for (const auto& caregiver : m_problem.caregivers())
    {
        routes.emplace_back(caregiver.GetId());
    }

    std::vector<int> customerIds(m_problem.customerCount());
    std::iota(customerIds.begin(), customerIds.end(), 1);
    std::shuffle(customerIds.begin(), customerIds.end(), m_rng);

    std::vector<int> centers = CenterIds(m_problem);
    std::unordered_map<int, int> centerLoad;

    for (int customerId : customerIds)
    {
        const Customer& customer = m_problem.GetCustomerById(customerId);
        std::shuffle(centers.begin(), centers.end(), m_rng);

        int selectedCenterId = -1;

        for (int centerId : centers)
        {
            if (centerLoad[centerId] >= m_problem.GetCenterById(centerId).GetCapacity())
            {
                continue;
            }

            bool hasSuitableCaregiver = false;

            for (const Caregiver& caregiver : m_problem.caregivers())
            {
                if (caregiver.GetCenterId() == centerId &&
                    caregiver.GetSkill() >= customer.GetRequiredSkill())
                {
                    hasSuitableCaregiver = true;
                    break;
                }
            }

            if (hasSuitableCaregiver)
            {
                selectedCenterId = centerId;
                break;
            }
        }

        if (selectedCenterId == -1)
        {
            InsertCustomerFeasible(routes, customerId);
            continue;
        }

        ++centerLoad[selectedCenterId];

        std::vector<int> candidateRouteIndexes;

        for (int i = 0; i < static_cast<int>(routes.size()); ++i)
        {
            const Caregiver& caregiver = m_problem.GetCaregiverById(routes[i].GetCaregiverId());
            if (caregiver.GetCenterId() == selectedCenterId) candidateRouteIndexes.push_back(i);
        }

        std::sort(candidateRouteIndexes.begin(), candidateRouteIndexes.end(),
            [&](int left, int right)
            {
                const Caregiver& a = m_problem.GetCaregiverById(routes[left].GetCaregiverId());
                const Caregiver& b = m_problem.GetCaregiverById(routes[right].GetCaregiverId());

                return a.GetSkill() < b.GetSkill();
            });

        int selectedRouteIndex = -1;

        for (int routeIndex : candidateRouteIndexes)
        {
            const Caregiver& caregiver = m_problem.GetCaregiverById(routes[routeIndex].GetCaregiverId());

            bool skillOk = caregiver.GetSkill() >= customer.GetRequiredSkill();
            bool workloadOk = routes[routeIndex].GetSize() < m_problem.maxWorkload();

            if (skillOk && workloadOk)
            {
                selectedRouteIndex = routeIndex;
                break;
            }
        }

        if (selectedRouteIndex == -1)
        {
            selectedRouteIndex = candidateRouteIndexes.empty()
                ? RandomIndex(routes.size(), m_rng)
                : candidateRouteIndexes[RandomIndex(candidateRouteIndexes.size(), m_rng)];
        }

        auto& routeCustomers = routes[selectedRouteIndex].GetCustomers();
        int position = routeCustomers.empty()
            ? 0
            : RandomIndex(routeCustomers.size() + 1, m_rng);

        routeCustomers.insert(routeCustomers.begin() + position, customerId);
    }

    return m_decoder.Encode(routes);
}

void Mobso::RemoveInfeasible(std::vector<Solution>& solutions) const
{
    solutions.erase(
        std::remove_if(
            solutions.begin(),
            solutions.end(),
            [&](const Solution& s)
            {
                return s.GetObjectives().serviceCost >= INFEASIBLE_PENALTY;
            }
        ),
        solutions.end()
    );
}

std::vector<Solution> Mobso::MakeChildren(std::vector<Solution> population)
{
    auto fronts = ParetoTools::AssignRanks(population);

    for (const auto& front : fronts)
    {
        ParetoTools::AssignCrowdingDistance(population, front);
    }

    std::vector<int> clusterCenters(fronts.size(), -1);

    for (size_t i = 0; i < fronts.size(); ++i)
    {
        if (!fronts[i].empty())
        {
            clusterCenters[i] = fronts[i][RandomIndex(fronts[i].size(), m_rng)];
        }
    }

    auto selectIndividual = [&](int clusterId, bool center) -> const Solution&
    {
        const std::vector<int>& front = fronts[clusterId];

        if (center || front.size() == 1)
        {
            return population[clusterCenters[clusterId]];
        }

        std::vector<int> normal;

        for (int index : front)
        {
            if (index != clusterCenters[clusterId])
            {
                normal.push_back(index);
            }
        }

        if (normal.empty())
        {
            return population[clusterCenters[clusterId]];
        }

        return population[normal[RandomIndex(normal.size(), m_rng)]];
    };

    std::vector<Solution> children;
    children.reserve(m_params.populationSize);

    while (static_cast<int>(children.size()) < m_params.populationSize)
    {
        if (fronts.size() > 1)
        {
            if (Random01(m_rng) < m_params.pg)
            {
                int clusterId = SelectFront(fronts);
                bool useCenter = Random01(m_rng) < m_params.po;

                const Solution& base = selectIndividual(clusterId, useCenter);
                children.push_back(Mutate(base));
            }
            else
            {
                int firstClusterId = SelectFront(fronts);
                int secondClusterId = SelectFront(fronts);
                bool useCenters = Random01(m_rng) < m_params.pt;

                const Solution& first =
                    selectIndividual(firstClusterId, useCenters);
                const Solution& second =
                    selectIndividual(secondClusterId, useCenters);

                children.push_back(Crossover(first, second));
            }
        }
        else
        {
            const auto& front = fronts.front();

            const Solution& first =
                population[front[RandomIndex(front.size(), m_rng)]];
            const Solution& second =
                population[front[RandomIndex(front.size(), m_rng)]];

            children.push_back(Crossover(first, second));
        }
    }

    return children;
}

Solution Mobso::Mutate(const Solution& parent) 
{
    auto routes = m_decoder.Decode(parent);
    std::vector<int> candidateRouteIndexes;

    for (int i = 0; i < static_cast<int>(routes.size()); ++i) 
    {
        if (routes[i].GetSize() >= 2) 
        {
            candidateRouteIndexes.push_back(i);
        }
    }

    if (candidateRouteIndexes.empty()) 
    {
        return parent;
    }

    Route& route = routes[candidateRouteIndexes[RandomIndex(candidateRouteIndexes.size(), m_rng)]];
    auto& customers = route.GetCustomers();

    const int from = RandomIndex(customers.size(), m_rng);
    const int to = RandomIndex(customers.size(), m_rng);

    if (Random01(m_rng) < 0.5) 
    {
        std::swap(customers[from], customers[to]);
    } 
    else 
    {
        const int customer = customers[from];
        customers.erase(customers.begin() + from);
        customers.insert(customers.begin() + to, customer);
    }

    return m_decoder.Encode(routes);
}

Solution Mobso::Crossover(const Solution& firstParent, const Solution& secondParent) 
{
    auto firstRoutes = m_decoder.Decode(firstParent);
    auto secondRoutes = m_decoder.Decode(secondParent);

    if (firstRoutes.empty() || secondRoutes.empty()) 
    {
        return firstParent;
    }
    const int caregiverId = firstRoutes[RandomIndex(firstRoutes.size(), m_rng)].GetCaregiverId();

    const Route& firstSelectedRoute = FindRouteByCaregiver(firstRoutes, caregiverId);
    const Route& secondSelectedRoute = FindRouteByCaregiver(secondRoutes, caregiverId);

    std::vector<int> onlyFirst = RouteDifference(firstSelectedRoute, secondSelectedRoute);
    std::vector<int> onlySecond = RouteDifference(secondSelectedRoute, firstSelectedRoute);
    RemoveCustomers(firstRoutes, onlyFirst);
    RemoveCustomers(secondRoutes, onlySecond);

    for (int customerId : onlySecond) 
    {
        InsertCustomerFeasible(firstRoutes, customerId);
    }

    for (int customerId : onlyFirst) 
    {
        InsertCustomerFeasible(secondRoutes, customerId);
    }

    Solution firstChild = m_decoder.Encode(firstRoutes);
    Solution secondChild = m_decoder.Encode(secondRoutes);

    firstChild.SetObjectives(m_evaluator.Evaluate(firstChild));
    secondChild.SetObjectives(m_evaluator.Evaluate(secondChild));
    if (Dominates(firstChild.GetObjectives(), secondChild.GetObjectives())) 
    {
        return firstChild;
    }

    if (Dominates(secondChild.GetObjectives(), firstChild.GetObjectives())) 
    {
        return secondChild;
    }

    return Random01(m_rng) < 0.5 ? firstChild : secondChild;
}

void Mobso::Evaluate(std::vector<Solution>& population) const 
{
    for (auto& solution : population) 
    {
        solution.SetObjectives(m_evaluator.Evaluate(solution));
    }
}

void Mobso::InsertCustomerFeasible(std::vector<Route>& routes, int customerId) 
{
    const Customer& customer = m_problem.GetCustomerById(customerId);
    std::vector<int> feasibleRouteIndexes;

    for (int i = 0; i < static_cast<int>(routes.size()); ++i) 
    {
        const Caregiver& caregiver = m_problem.GetCaregiverById(routes[i].GetCaregiverId());

        const bool skillOk = caregiver.GetSkill() >= customer.GetRequiredSkill();
        const bool workloadOk = routes[i].GetSize() < m_problem.maxWorkload();

        if (skillOk && workloadOk) feasibleRouteIndexes.push_back(i);
    }

    const int routeId = feasibleRouteIndexes.empty()
        ? RandomIndex(routes.size(), m_rng)
        : feasibleRouteIndexes[RandomIndex(feasibleRouteIndexes.size(), m_rng)];

    auto& customers = routes[routeId].GetCustomers();
    const int position = customers.empty() ? 0 : RandomIndex(customers.size() + 1, m_rng);
    customers.insert(customers.begin() + position, customerId);
}

int Mobso::SelectFront(const std::vector<std::vector<int>>& fronts) 
{
    const int first = RandomIndex(fronts.size(), m_rng);
    const int second = RandomIndex(fronts.size(), m_rng);
    return std::min(first, second);
}

const Solution& Mobso::SelectFromFront(
    const std::vector<Solution>& population,
    const std::vector<int>& front) 
{

    if (Random01(m_rng) < m_params.po) 
    {
        int best = front.front();

        for (int index : front) 
        {
            if (population[index].GetCrowdingDistance() > population[best].GetCrowdingDistance()) 
            {
                best = index;
            }
        }

        return population[best];
    }
    return population[front[RandomIndex(front.size(), m_rng)]];
}

std::vector<int> Mobso::RouteDifference(const Route& left, const Route& right) 
{
    std::unordered_set<int> rightCustomers(
        right.GetCustomers().begin(),
        right.GetCustomers().end()
    );

    std::vector<int> result;

    for (int customerId : left.GetCustomers()) 
    {
        if (!rightCustomers.count(customerId)) 
        {
            result.push_back(customerId);
        }
    }

    return result;
}

void Mobso::RemoveCustomers(std::vector<Route>& routes, const std::vector<int>& customers) 
{
    std::unordered_set<int> customersToRemove(customers.begin(), customers.end());

    for (auto& route : routes) 
    {
        auto& routeCustomers = route.GetCustomers();

        routeCustomers.erase(
            std::remove_if(
                routeCustomers.begin(),
                routeCustomers.end(),
                [&](int customerId) 
                {
                    return customersToRemove.count(customerId) > 0;
                }
            ),
            routeCustomers.end()
        );
    }
}
