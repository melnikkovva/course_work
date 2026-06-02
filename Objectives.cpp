#include "Objectives.h"

bool Dominates(const Objectives& left, const Objectives& right) 
{
    const bool noWorse = left.serviceCost <= right.serviceCost && left.delayCost <= right.delayCost;
    const bool strictlyBetter = left.serviceCost < right.serviceCost || left.delayCost < right.delayCost;

    return noWorse && strictlyBetter;
}