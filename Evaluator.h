#pragma once

#include "Decoder.h"
#include "Objectives.h"
#include "Problem.h"
#include "Solution.h"

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