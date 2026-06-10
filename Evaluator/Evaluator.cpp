#include "Evaluator.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>
#include "../constants.h"

Evaluator::Evaluator(const Problem& problem)
    : m_problem(problem), m_decoder(problem) {}

Objectives Evaluator::Evaluate(const Solution& solution) const
{
    Objectives result;

    std::vector<int> visits(m_problem.customerCount() + 1, 0);
    std::unordered_map<int, int> centerLoad;

    std::vector<Route> routes = m_decoder.Decode(solution);

    for (const Route& route : routes)
    {
        EvaluateRoute(route, result, visits, centerLoad);
    }

    AddMissingOrDuplicateVisitPenalty(result, visits);
    AddCenterCapacityPenalty(result, centerLoad);

    return result;
}

void Evaluator::EvaluateRoute(
    const Route& route,
    Objectives& result,
    std::vector<int>& visits,
    std::unordered_map<int, int>& centerLoad) const
{
    const Caregiver& caregiver = m_problem.GetCaregiverById(route.GetCaregiverId());

    AddWorkloadPenalty(route, result);

    double time = 0.0;
    int previousCustomerId = -1;

    for (int customerId : route.GetCustomers())
    {
        EvaluateCustomerVisit(customerId, caregiver,
            previousCustomerId, time, result,
            visits, centerLoad
        );
    }
}

void Evaluator::AddWorkloadPenalty(
    const Route& route,
    Objectives& result) const
{
    if (route.GetSize() > m_problem.maxWorkload())
    {
        result.serviceCost += INFEASIBLE_PENALTY * (route.GetSize() - m_problem.maxWorkload());
    }
}

void Evaluator::EvaluateCustomerVisit(
    int customerId,
    const Caregiver& caregiver,
    int& previousCustomerId,
    double& time,
    Objectives& result,
    std::vector<int>& visits,
    std::unordered_map<int, int>& centerLoad) const
{
    const Customer& customer = m_problem.GetCustomerById(customerId);

    visits[customerId]++;
    centerLoad[caregiver.GetCenterId()]++;

    if (caregiver.GetSkill() < customer.GetRequiredSkill())
    {
        result.serviceCost += INFEASIBLE_PENALTY;
    }

    if (previousCustomerId == -1)
    {
        time += m_problem.CountWayCenterCustomer(
            caregiver.GetCenterId(),
            customerId
        );
    }
    else
    {
        time += m_problem.CountWayCustomerCustomer(
            previousCustomerId,
            customerId
        );
    }

    const double skillMultiplier = static_cast<double>(caregiver.GetSkill())
                                 / static_cast<double>(customer.GetRequiredSkill());

    result.serviceCost += customer.GetServiceTime() * skillMultiplier * customer.GetBaseRate();
    result.delayCost += m_problem.delayPenalty() * std::max(0.0, time - customer.GetAppointmentTime());

    time += customer.GetServiceTime();
    previousCustomerId = customerId;
}

void Evaluator::AddMissingOrDuplicateVisitPenalty(
    Objectives& result,
    const std::vector<int>& visits) const
{
    for (int id = 1; id <= m_problem.customerCount(); ++id)
    {
        if (visits[id] != 1)
        {
            result.serviceCost += INFEASIBLE_PENALTY * std::abs(visits[id] - 1);
        }
    }
}

void Evaluator::AddCenterCapacityPenalty(
    Objectives& result,
    const std::unordered_map<int, int>& centerLoad) const
{
    for (const CareCenter& center : m_problem.centers())
    {
        auto it = centerLoad.find(center.GetId());
        int load = 0;

        if (it != centerLoad.end())
        {
            load = it->second;
        }

        if (load > center.GetCapacity())
        {
            result.serviceCost += INFEASIBLE_PENALTY * (load - center.GetCapacity());
        }
    }
}