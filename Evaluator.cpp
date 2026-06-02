#include "Evaluator.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

Evaluator::Evaluator(const Problem& problem)
    : m_problem(problem), m_decoder(problem) {}

Objectives Evaluator::Evaluate(const Solution& solution) const 
{
    Objectives result;
    std::vector<int> visits(m_problem.customerCount() + 1, 0);
    std::unordered_map<int, int> centerLoad;

    for (const auto& route : m_decoder.Decode(solution)) 
    {
        const auto& caregiver = m_problem.GetCaregiverById(route.GetCaregiverId());

        if (route.GetSize() > m_problem.maxWorkload()) 
        {
            result.serviceCost += InfeasiblePenalty * (route.GetSize() - m_problem.maxWorkload());
        }

        double time = 0.0;
        int previousCustomerId = -1;

        for (int customerId : route.GetCustomers()) 
        {
            const auto& customer = m_problem.GetCustomerById(customerId);

            visits[customerId]++;
            centerLoad[caregiver.GetCenterId()]++;
            if (caregiver.GetSkill() < customer.GetRequiredSkill()) 
            {
                result.serviceCost += InfeasiblePenalty;
            }

            time += previousCustomerId == -1
                ? m_problem.CountWayCenterCustomer(caregiver.GetCenterId(), customerId)
                : m_problem.CountWayCustomerCustomer(previousCustomerId, customerId);

            const double skillMultiplier =
                static_cast<double>(caregiver.GetSkill()) /
                static_cast<double>(customer.GetRequiredSkill());

            result.serviceCost += customer.GetServiceTime() * skillMultiplier * customer.GetBaseRate();

            result.delayCost += m_problem.delayPenalty() * std::max(0.0, time - customer.GetAppointmentTime());

            time += customer.GetServiceTime();
            previousCustomerId = customerId;
        }
    }

    for (int id = 1; id <= m_problem.customerCount(); ++id) 
    {
        if (visits[id] != 1) 
        {
            result.serviceCost += InfeasiblePenalty * std::abs(visits[id] - 1);
        }
    }

    for (const auto& center : m_problem.centers()) 
    {
        if (centerLoad[center.GetId()] > center.GetCapacity()) 
        {
            result.serviceCost += InfeasiblePenalty * (centerLoad[center.GetId()] - center.GetCapacity());
        }
    }

    return result;
}
