#include "BasicTests.h"
#include <cmath>
#include <iostream>
#include <vector>
#include "../CareCenter/CareCenter.h"
#include "../CareGiver/CareGiver.h"
#include "../Customer/Customer.h"
#include "../Decoder/Decoder.h"
#include "../Evaluator/Evaluator.h"
#include "../Mobso/Mobso.h"
#include "../Problem/Problem.h"
#include "../Solution/Solution.h"

bool HasBigPenalty(const Objectives& obj)
{
    return obj.serviceCost > 1e8;
}

bool IsCorrectNumber(const Objectives& obj)
{
    return std::isfinite(obj.serviceCost) && std::isfinite(obj.delayCost);
}

bool HasAllCustomersOnce(
    const Problem& problem,
    const Solution& solution)
{
    Decoder decoder(problem);

    std::vector<int> count(problem.customerCount() + 1, 0);

    for (const Route& route : decoder.Decode(solution)) 
    {
        for (int customerId : route.GetCustomers()) 
        {
            if (customerId < 1 || customerId > problem.customerCount()) 
            {
                return false;
            }

            count[customerId]++;
        }
    }

    for (int id = 1; id <= problem.customerCount(); ++id) 
    {
        if (count[id] != 1) 
        {
            return false;
        }
    }

    return true;
}

void PrintTest(
    const std::string& name,
    bool ok)
{
    std::cout << (ok ? "[OK]   " : "[FAIL] ") << name << '\n';
}

Problem CreateSmallProblem()
{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 0, 0, 10)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 1, 0, 1, 10, 100, 1),
        Customer(2, 2, 0, 2, 10, 100, 1),
        Customer(3, 3, 0, 3, 10, 100, 1)
    };

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(4, 1, 1),
        Caregiver(5, 1, 2),
        Caregiver(6, 1, 3)
    };

    return Problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        5,
        1.0
    );
}

bool TestBasic()
{
    Problem problem = CreateSmallProblem();

    Mobso::Params params;
    params.populationSize = 20;
    params.maxEvaluations = 20;
    params.seed = 1;

    Mobso mobso(problem, params);
    std::vector<Solution> result = mobso.run();

    return !result.empty() &&
           IsCorrectNumber(result[0].GetObjectives()) &&
           HasAllCustomersOnce(problem, result[0]);
}

bool TestFewPatients()
{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 0, 0, 2),
        CareCenter(2, 10, 0, 2)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 1, 0, 1, 5, 100, 1),
        Customer(2, 9, 0, 1, 5, 100, 1)
    };

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(3, 1, 1),
        Caregiver(4, 2, 1)
    };

    Problem problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        3,
        1.0
    );

    Mobso::Params params;
    params.populationSize = 20;
    params.maxEvaluations = 20;
    params.seed = 2;

    Mobso mobso(problem, params);
    std::vector<Solution> result = mobso.run();

    return !result.empty() &&
           IsCorrectNumber(result[0].GetObjectives()) &&
           HasAllCustomersOnce(problem, result[0]);
}

bool TestCenterCapacity()

{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 0, 0, 1)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 1, 0, 1, 5, 100, 1),
        Customer(2, 2, 0, 1, 5, 100, 1)
    };

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(3, 1, 1)
    };

    Problem problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        5,
        1.0
    );

    Solution solution({3, 1, 2});
    Evaluator evaluator(problem);
    Objectives obj = evaluator.Evaluate(solution);

    return HasBigPenalty(obj);
}

bool TestHighSkill()
{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 0, 0, 10)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 1, 0, 3, 5, 100, 1)
    };

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(2, 1, 3)
    };

    Problem problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        5,
        1.0
    );

    Solution solution(
        {2, 1});

    Evaluator evaluator(problem);
    Objectives obj = evaluator.Evaluate(solution);

    return !HasBigPenalty(obj);
}

bool TestAllSkillsTooLow()
{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 0, 0, 10)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 1, 0, 3, 5, 100, 1)
    };

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(2, 1, 1)
    };

    Problem problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        5,
        1.0
    );

    Solution solution(
        {2, 1});

    Evaluator evaluator(problem);
    Objectives obj = evaluator.Evaluate(solution);

    return HasBigPenalty(obj);
}

bool TestWorkloadLimit()
{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 0, 0, 10)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 1, 0, 1, 5, 100, 1),
        Customer(2, 2, 0, 1, 5, 100, 1)
    };

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(3, 1, 1)
    };

    Problem problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        1,
        1.0
    );

    Solution solution(
        {3, 1, 2});

    Evaluator evaluator(problem);
    Objectives obj = evaluator.Evaluate(solution);

    return HasBigPenalty(obj);
}

bool TestWorkloadLessThanPatients()
{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 0, 0, 10)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 1, 0, 1, 5, 100, 1),
        Customer(2, 2, 0, 1, 5, 100, 1),
        Customer(3, 3, 0, 1, 5, 100, 1)
    };

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(4, 1, 1)
    };

    Problem problem(std::move(customers), std::move(centers),
                    std::move(caregivers), 2, 1.0);

    Mobso::Params params;
    params.populationSize = 20;
    params.maxEvaluations = 20;
    params.seed = 3;

    Mobso mobso(problem, params);
    std::vector<Solution> result = mobso.run();

    return !result.empty() &&
        IsCorrectNumber(result[0].GetObjectives()) &&
           HasBigPenalty(result[0].GetObjectives());
}

bool TestSameCoordinates()
{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 0, 0, 10)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 1, 1, 1, 5, 100, 1),
        Customer(2, 1, 1, 1, 5, 100, 1)
    };

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(3, 1, 1)
    };

    Problem problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        5,
        1.0
    );

    Solution solution(
        {3, 1, 2});

    Evaluator evaluator(problem);
    Objectives obj = evaluator.Evaluate(solution);

    return IsCorrectNumber(obj);
}

bool TestQ1PreferredOverQ3()
{
    std::vector<CareCenter> centers = 
    {
        CareCenter(1, 0, 0, 10)
    };

    std::vector<Customer> customers = 
    {
        Customer(1, 1, 0, 1, 10, 100, 1)
    };

    std::vector<Caregiver> caregivers = 
    {
        Caregiver(2, 1, 1),
        Caregiver(3, 1, 3)
    };

    Problem problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        5,
        1.0
    );

    Evaluator evaluator(problem);

    Solution q1Solution(
        {2, 1, 3});
    Solution q3Solution(
        {2, 3, 1});

    Objectives q1 = evaluator.Evaluate(q1Solution);
    Objectives q3 = evaluator.Evaluate(q3Solution);

    return q1.serviceCost < q3.serviceCost;
}

bool TestMutationAndCrossover()
{
    Problem problem = CreateSmallProblem();

    Mobso::Params params;
    params.populationSize = 40;
    params.maxEvaluations = 40;
    params.seed = 4;

    Mobso mobso(problem, params);
    std::vector<Solution> result = mobso.run();

    if (result.empty()) 
    {
        return false;
    }

    for (const Solution& solution : result) 
    {
        if (!HasAllCustomersOnce(problem, solution)) 
        {
            return false;
        }

        if (!IsCorrectNumber(solution.GetObjectives())) 
        {
            return false;
        }
    }

    return true;
}

void RunBasicTests()
{
    PrintTest("1. Basic correctness", TestBasic());
    PrintTest("2. Several centers and few patients", TestFewPatients());
    PrintTest("3. Center capacity", TestCenterCapacity());
    PrintTest("4. High skill requirement", TestHighSkill());
    PrintTest("5. All skills too low", TestAllSkillsTooLow());
    PrintTest("6. Caregiver workload limit", TestWorkloadLimit());
    PrintTest("7. Workload less than patients", TestWorkloadLessThanPatients());
    PrintTest("8. Same coordinates", TestSameCoordinates());
    PrintTest("9. Q1 preferred over Q3", TestQ1PreferredOverQ3());
    PrintTest("10. Mutation and crossover validity", TestMutationAndCrossover());
}