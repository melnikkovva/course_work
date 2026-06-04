#pragma once
#include <cstdint>
#include <random>
#include <vector>
#include "../Decoder/Decoder.h"
#include "../Evaluator/Evaluator.h"
#include "../Problem/Problem.h"
#include "../Route/Route.h"
#include "../Solution/Solution.h"

class Mobso 
{
public:
    struct Params 
    {
        int populationSize = 80;
        int maxFitnessEvaluationsMultiplier = 100;
        double pg = 0.8;
        double po = 0.6;
        double pt = 0.4;
        std::uint32_t seed = std::random_device{}();
    };

    Mobso(const Problem& problem, Params params);
    std::vector<Solution> run();

private:
    const Problem& m_problem;
    Params m_params;
    std::mt19937 m_rng;
    Evaluator m_evaluator;
    Decoder m_decoder;

    std::vector<Solution> InitializePopulation();
    Solution MakeInitialSolution();
    std::vector<Solution> MakeChildren(std::vector<Solution> population);
    Solution Mutate(const Solution& parent);
    Solution Crossover(const Solution& firstParent, const Solution& secondParent);
    void Evaluate(std::vector<Solution>& population) const;
    void InsertCustomerFeasible(std::vector<Route>& routes, int customerId);
    int SelectFront(const std::vector<std::vector<int>>& fronts);
    const Solution& SelectFromFront(const std::vector<Solution>& population, const std::vector<int>& front);
    double Random01();
    int RandomIndex(size_t size);
    static std::vector<int> RouteDifference(const Route& left, const Route& right);
    static void RemoveCustomers(std::vector<Route>& routes, const std::vector<int>& customers);
};