#pragma once

struct Objectives 
{
    double serviceCost = 0.0;
    double delayCost = 0.0;
};

bool Dominates(const Objectives& left, const Objectives& right);