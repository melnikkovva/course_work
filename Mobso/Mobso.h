#pragma once
#include <cstdint>
#include <random>
#include <vector>
#include "../Decoder/Decoder.h"
#include "../Evaluator/Evaluator.h"
#include "../Problem/Problem.h"
#include "../Route/Route.h"
#include "../Solution/Solution.h"
#include "../constants.h"

class Mobso 
{
public:
    struct Params 
    {
        int populationSize = POPULATION_SIZE;
        int maxEvaluations = MAX_EVALUATIONS;
        double pg = PG;
        double po = PO;
        double pt = PT;
        std::uint32_t seed = RANDOM_SEED;
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
    Solution MakeFeasibleInitialSolution();
    Solution MakeRandomInitialSolution();
    void RemoveInfeasible(std::vector<Solution>& solutions) const;
    std::vector<Solution> MakeChildren(std::vector<Solution> population);
    Solution Mutate(const Solution& parent);
    Solution Crossover(const Solution& firstParent, const Solution& secondParent);
    void Evaluate(std::vector<Solution>& population) const;
    void InsertCustomerFeasible(std::vector<Route>& routes, int customerId);
    int SelectFront(const std::vector<std::vector<int>>& fronts);
    const Solution& SelectFromFront(const std::vector<Solution>& population, const std::vector<int>& front);
    void Mobso::LogPopulation(
    const std::string& title,
    const std::vector<Solution>& population) const;
    static std::vector<int> RouteDifference(const Route& left, const Route& right);
    static void RemoveCustomers(std::vector<Route>& routes, const std::vector<int>& customers);
};