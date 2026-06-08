#include <iostream>

#include "Decoder/Decoder.h"
#include "Mobso/Mobso.h"
#include "Tests/BasicTests.h"
#include "Tests/TestLineProblem.h"
#include "Tests/TestBigProblem.h"
#include "Visualizer/Visualizer.h"

void PrintSolution(const Problem& problem, const Solution& solution)
{
    Decoder decoder(problem);

    std::cout
        << "Service cost = " << solution.GetObjectives().serviceCost
        << "\nDelay cost = " << solution.GetObjectives().delayCost << "\n\n";

    std::vector<Route> routes = decoder.Decode(solution);

    for (const Route& route : routes)
    {
        const Caregiver& caregiver =
            problem.GetCaregiverById(route.GetCaregiverId());

        for (int customerId : route.GetCustomers())
        {
            std::cout << "C" << customerId << " ";
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

void RunProblem(const Problem& problem)
{
    Mobso::Params params;

    params.populationSize = 100;
    params.maxEvaluations = 10000;
    params.pg = 0.2;
    params.po = 0.6;
    params.pt = 0.2;
    params.seed = 42;

    Mobso optimizer(problem, params);

    std::vector<Solution> paretoFront = optimizer.run();

    std::cout << "Pareto solutions found: " << paretoFront.size() << "\n\n";

    for (size_t i = 0; i < paretoFront.size(); ++i)
    {
        std::cout << "Solution " << i + 1 << "\n";
        PrintSolution(problem, paretoFront[i]);
    }


    Visualizer::Show(problem, paretoFront);
}

int main()
{
    Problem bigProblem = CreateBigProblem();

    RunProblem(bigProblem);

    return 0;
}