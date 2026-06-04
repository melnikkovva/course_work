#include "MOBSO.h"
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include "ParetoTools.h"

Mobso::Mobso(const Problem& problem, Params params)
    : m_problem(problem),
      m_params(params),
      m_rng(params.seed),
      m_evaluator(problem),
      m_decoder(problem) {}

      std::vector<Solution> Mobso::run() 
    {
    std::vector<Solution> population = InitializePopulation();
    Evaluate(population);

    std::vector<Solution> archive = ParetoTools::NonDominated(population);

    const int maxEvaluations =
        m_params.maxFitnessEvaluationsMultiplier * m_problem.customerCount();

    int evaluations = static_cast<int>(population.size());

    while (evaluations < maxEvaluations) {
        std::vector<Solution> children = MakeChildren(population);
        Evaluate(children);
        evaluations += static_cast<int>(children.size());

        population.insert(population.end(), children.begin(), children.end());
        population = ParetoTools::SelectBest(population, m_params.populationSize);

        archive.insert(archive.end(), population.begin(), population.end());
        archive = ParetoTools::NonDominated(archive);
    }

    return archive;
}

std::vector<Solution> Mobso::InitializePopulation() 
{
    std::vector<Solution> population;
    population.reserve(m_params.populationSize);

    for (int i = 0; i < m_params.populationSize; ++i) 
    {
        population.push_back(MakeInitialSolution());
    }

    return population;
}

Solution Mobso::MakeInitialSolution() 
{
    std::vector<int> customerIds(m_problem.customerCount());
    std::iota(customerIds.begin(), customerIds.end(), 1);
    std::shuffle(customerIds.begin(), customerIds.end(), m_rng);

    std::vector<Route> routes;
    routes.reserve(m_problem.caregivers().size());

    for (const auto& caregiver : m_problem.caregivers()) 
    {
        routes.emplace_back(caregiver.GetId());
    }

    std::sort(routes.begin(), routes.end(), [&](const Route& left, const Route& right) 
    {
        return m_problem.GetCaregiverById(left.GetCaregiverId()).GetSkill() <
               m_problem.GetCaregiverById(right.GetCaregiverId()).GetSkill();
    });

    for (int customerId : customerIds) 
    {
        InsertCustomerFeasible(routes, customerId);
    }

    return m_decoder.Encode(routes);
}

std::vector<Solution> Mobso::MakeChildren(std::vector<Solution> population) 
{
    auto fronts = ParetoTools::AssignRanks(population);

    for (const auto& front : fronts) 
    {
        ParetoTools::AssignCrowdingDistance(population, front);
    }

    std::vector<Solution> children;
    children.reserve(m_params.populationSize);

    while (static_cast<int>(children.size()) < m_params.populationSize) 
    {
        const bool useOneCluster = fronts.size() > 1 && Random01() < m_params.pg;

        if (useOneCluster) 
        {
            const int frontId = SelectFront(fronts);
            const Solution& base = SelectFromFront(population, fronts[frontId]);
            children.push_back(Mutate(base));
        } 
        else 
        {
            const Solution& first = population[RandomIndex(population.size())];
            const Solution& second = population[RandomIndex(population.size())];
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

    Route& route = routes[candidateRouteIndexes[RandomIndex(candidateRouteIndexes.size())]];
    auto& customers = route.GetCustomers();

    const int from = RandomIndex(customers.size());
    const int to = RandomIndex(customers.size());

    if (Random01() < 0.5) 
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

    const int routeId = RandomIndex(std::min(firstRoutes.size(), secondRoutes.size()));

    std::vector<int> onlyFirst = RouteDifference(firstRoutes[routeId], secondRoutes[routeId]);
    std::vector<int> onlySecond = RouteDifference(secondRoutes[routeId], firstRoutes[routeId]);

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

    return Random01() < 0.5 ? firstChild : secondChild;
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
        ? RandomIndex(routes.size())
        : feasibleRouteIndexes[RandomIndex(feasibleRouteIndexes.size())];

    auto& customers = routes[routeId].GetCustomers();
    const int position = customers.empty() ? 0 : RandomIndex(customers.size() + 1);
    customers.insert(customers.begin() + position, customerId);
}

int Mobso::SelectFront(const std::vector<std::vector<int>>& fronts) 
{
    const int first = RandomIndex(fronts.size());
    const int second = RandomIndex(fronts.size());
    return std::min(first, second);
}

const Solution& Mobso::SelectFromFront(
    const std::vector<Solution>& population,
    const std::vector<int>& front) 
{

    if (Random01() < m_params.po) 
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
    return population[front[RandomIndex(front.size())]];
}

double Mobso::Random01() 
{
    return std::uniform_real_distribution<double>(0.0, 1.0)(m_rng);
}

int Mobso::RandomIndex(size_t size) 
{
    return std::uniform_int_distribution<int>(0, static_cast<int>(size) - 1)(m_rng);
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
