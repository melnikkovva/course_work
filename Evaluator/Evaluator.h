#pragma once
#include <limits>
#include "../Decoder/Decoder.h"
#include "../Objectives/Objectives.h"
#include "../Problem/Problem.h"
#include "../Solution/Solution.h"
#include <unordered_map>
#include <vector>
#include "../Route/Route.h"
#include "../CareGiver/CareGiver.h"

class Evaluator 
{
public:
    explicit Evaluator(const Problem& problem);
    Objectives Evaluate(const Solution& solution) const;
private:
    const Problem& m_problem;
    Decoder m_decoder;
    
    void EvaluateRoute(
        const Route& route,
        Objectives& result,
        std::vector<int>& visits,
        std::unordered_map<int, int>& centerLoad
    ) const;

    void AddWorkloadPenalty(
        const Route& route,
        Objectives& result
    ) const;

    void EvaluateCustomerVisit(
        int customerId,
        const Caregiver& caregiver,
        int& previousCustomerId,
        double& time,
        Objectives& result,
        std::vector<int>& visits,
        std::unordered_map<int, int>& centerLoad
    ) const;

    void AddMissingOrDuplicateVisitPenalty(
        Objectives& result,
        const std::vector<int>& visits
    ) const;

    void AddCenterCapacityPenalty(
        Objectives& result,
        const std::unordered_map<int, int>& centerLoad
    ) const;
};