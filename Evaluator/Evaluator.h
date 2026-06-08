#pragma once
#include <limits>
#include "../Decoder/Decoder.h"
#include "../Objectives/Objectives.h"
#include "../Problem/Problem.h"
#include "../Solution/Solution.h"

class Evaluator 
{
public:
    explicit Evaluator(const Problem& problem);
    Objectives Evaluate(const Solution& solution) const;

private:
    static constexpr double InfeasiblePenalty = 1e9;

    const Problem& m_problem;
    Decoder m_decoder;
};