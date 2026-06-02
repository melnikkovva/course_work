#include <iostream>
#include <vector>
#include "CareCenter.h"
#include "Caregiver.h"
#include "Customer.h"
#include "Mobso.h"
#include "Problem.h"

int main() 
{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 53.5, 69.5, 20),
        CareCenter(2, 66.0, 68.0, 20)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 53.8, 66.8, 1, 20, 48, 1.0),
        Customer(2, 49.4, 65.9, 1, 20, 64, 1.0),
        Customer(3, 70.6, 69.8, 1, 20, 39, 1.0),
        Customer(4, 57.9, 69.0, 1, 20, 32, 1.0),
        Customer(5, 66.1, 63.4, 1, 20, 43, 1.0),
        Customer(6, 50.8, 72.0, 1, 20, 50, 1.0),
        Customer(7, 65.6, 72.2, 1, 20, 57, 1.0),
        Customer(8, 47.1, 63.3, 1, 20, 61, 1.0),
        Customer(9, 57.4, 65.7, 1, 20, 62, 1.0),
        Customer(10, 71.2, 64.8, 1, 20, 47, 1.0)
    };

    const int n = static_cast<int>(customers.size());

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(n + 1, 1, 1),
        Caregiver(n + 2, 1, 2),
        Caregiver(n + 3, 1, 3),
        Caregiver(n + 4, 2, 1),
        Caregiver(n + 5, 2, 2),
        Caregiver(n + 6, 2, 3)
    };

    Problem problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        5,
        1.0
    );

    Mobso::Params params;
    params.populationSize = 80;
    params.maxFitnessEvaluationsMultiplier = 100;
    params.pg = 0.8;
    params.po = 0.6;
    params.pt = 0.4;
    params.seed = 42;
    Mobso optimizer(problem, params);
    const auto paretoFront = optimizer.run();

    std::cout << "Pareto solutions: " << paretoFront.size() << '\n';

    for (const auto& solution : paretoFront) 
    {
        std::cout
            << "serviceCost=" << solution.GetObjectives().serviceCost
            << ", delayCost=" << solution.GetObjectives().delayCost
            << '\n';
    }

    return 0;
}

