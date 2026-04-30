#include "MOBSO.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

MOBSO::MOBSO(std::vector<Patient> patients, std::vector<CareCenter> centers) :
    m_patients(std::move(patients)), m_centers(std::move(centers))
{
    for (const auto& center : m_centers) 
    {
        for (const auto& nurse : center.nurses) 
        {
            m_nurses.push_back(nurse);
        }
    }
}

int MOBSO::SkillValue(SkillLevel skill) const
{
    return static_cast<int>(skill);
}

bool MOBSO::HasEnoughSkill(SkillLevel caregiverSkill, SkillLevel requiredSkill) const
{
    return SkillValue(caregiverSkill) >= SkillValue(requiredSkill);
}

const Patient* MOBSO::FindPatient(int id) const
{
    auto it = std::find_if(m_patients.begin(), m_patients.end(), [&](const Patient& patient) 
    {
        return patient.id == id;
    });

    return it == m_patients.end() ? nullptr : &(*it);
}

const Nurse* MOBSO::FindNurse(int id) const
{
    auto it = std::find_if(m_nurses.begin(), m_nurses.end(), [&](const Nurse& nurse) 
    {
        return nurse.id == id;
    });

    return it == m_nurses.end() ? nullptr : &(*it);
}

bool MOBSO::IsPatientId(int id) const
{
    return FindPatient(id) != nullptr;
}

bool MOBSO::IsNurseId(int id) const
{
    return FindNurse(id) != nullptr;
}

DecodedSolution MOBSO::SplitByNurse(const std::vector<int>& code) const
{
    DecodedSolution decoded;
    int currentIndex = -1;

    for (int id : code) 
    {
        if (IsNurseId(id)) 
        {
            decoded.push_back({id, {}});
            currentIndex = static_cast<int>(decoded.size()) - 1;
        } 
        else if (IsPatientId(id) && currentIndex >= 0) 
        {
            decoded[currentIndex].second.push_back(id);
        }
    }

    return decoded;
}

std::vector<int> MOBSO::BuildCode(const DecodedSolution& decoded) const
{
    std::vector<int> code;

    for (const auto& item : decoded) 
    {
        code.push_back(item.first);
        code.insert(code.end(), item.second.begin(), item.second.end());
    }

    return code;
}

bool MOBSO::CanAssignPatientToNurse(int nurseId, int patientId, int currentWorkload) const
{
    const Patient* patient = FindPatient(patientId);
    const Nurse* nurse = FindNurse(nurseId);

    if (patient == nullptr || nurse == nullptr) 
    {
        return false;
    }

    return HasEnoughSkill(nurse->skill, patient->requiredSkill) && currentWorkload < nurse->maxWorkload;
}

bool MOBSO::IsDominate(const Solution& a, const Solution& b) const
{
    const bool notWorse = a.costs.serviceCost <= b.costs.serviceCost
                         && a.costs.delayCost <= b.costs.delayCost;

    const bool strictlyBetter = a.costs.serviceCost < b.costs.serviceCost 
                                || a.costs.delayCost < b.costs.delayCost;
    return notWorse && strictlyBetter;
}

std::vector<Cluster> MOBSO::CreateClusters(std::vector<Solution> population) const
{
    std::vector<Cluster> clusters;
    int currentRank = 0;

    while (!population.empty()) 
    {
        Cluster front;

        for (const auto& candidate : population) 
        {
            bool dominatedByAny = false;

            for (const auto& other : population) 
            {
                if (&candidate != &other && IsDominate(other, candidate)) 
                {
                    dominatedByAny = true;
                    break;
                }
            }

            if (!dominatedByAny) 
            {
                Solution ranked = candidate;
                ranked.rank = currentRank;
                front.push_back(ranked);
            }
        }
        if (front.empty()) 
        {
            break;
        }
        clusters.push_back(front);
        population.erase(
            std::remove_if(population.begin(), population.end(), [&](const Solution& solution) 
            {
                return std::any_of(front.begin(), front.end(), [&](const Solution& selected) 
                {
                    return solution.code == selected.code;
                });
            }),
            population.end()
        );
        ++currentRank;
    }

    return clusters;
}

std::vector<int> MOBSO::FindMutableRoutes(const DecodedSolution& decoded) const
{
    std::vector<int> indexes;

    for (int i = 0; i < static_cast<int>(decoded.size()); ++i) 
    {
        if (decoded[i].second.size() >= 2) 
        {
            indexes.push_back(i);
        }
    }

    return indexes;
}

void MOBSO::Mutate(Solution& solution)
{
    auto decoded = SplitByNurse(solution.code);
    std::vector<int> indexes = FindMutableRoutes(decoded);

    if (indexes.empty()) return;

    std::uniform_int_distribution<int> routeDist(0, static_cast<int>(indexes.size()) - 1);

    int routeIndex = indexes[routeDist(m_gen)];
    std::vector<int>& route = decoded[routeIndex].second;

    std::uniform_int_distribution<int> patientDist(0, static_cast<int>(route.size()) - 1);

    int first = patientDist(m_gen);
    int second = patientDist(m_gen);

    while (second == first) 
    {
        second = patientDist(m_gen);
    }

    std::swap(route[first], route[second]);

    solution.code = BuildCode(decoded);
}

int MOBSO::SelectRandomNurseId(const DecodedSolution& decoded)
{
    std::uniform_int_distribution<int> nurseDist(0, static_cast<int>(decoded.size()) - 1);
    return decoded[nurseDist(m_gen)].first;
}

std::vector<int>* MOBSO::FindRouteByNurseId(DecodedSolution& decoded, int nurseId) const
{
    for (auto& route : decoded) 
    {
        if (route.first == nurseId) 
        {
            return &route.second;
        }
    }

    return nullptr;
}

std::vector<int> MOBSO::GetDifferentPatients(const std::vector<int>& first, const std::vector<int>& second) const
{
    std::vector<int> differentPatients;

    for (int patientId : first) 
    {
        if (std::find(second.begin(), second.end(), patientId) == second.end()) 
        {
            differentPatients.push_back(patientId);
        }
    }

    return differentPatients;
}

void MOBSO::RemovePatientsFromDecoded(DecodedSolution& decoded,
                                      const std::vector<int>& patients) const
{
    for (auto& route : decoded) 
    {
        route.second.erase(
            std::remove_if(route.second.begin(), route.second.end(), [&](int patientId) 
            {
                return std::find(patients.begin(), patients.end(), patientId) != patients.end();
            }),
            route.second.end()
        );
    }
}

bool MOBSO::TryInsertPatient(DecodedSolution& decoded, int patientId)
{
    std::vector<int> order(decoded.size());
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), m_gen);

    for (int index : order) 
    {
        if (CanAssignPatientToNurse(decoded[index].first, patientId, static_cast<int>(decoded[index].second.size()))) 
        {
            decoded[index].second.push_back(patientId);
            return true;
        }
    }

    return false;
}

void MOBSO::InsertPatients(DecodedSolution& decoded, const std::vector<int>& patients)
{
    for (int patientId : patients) 
    {
        TryInsertPatient(decoded, patientId);
    }
}

Solution MOBSO::Crossover(const Solution& parent1, const Solution& parent2)
{
    Solution child = parent1;

    auto decoded1 = SplitByNurse(parent1.code);
    auto decoded2 = SplitByNurse(parent2.code);

    if (decoded1.empty() || decoded2.empty()) 
    {
        return child;
    }

    int nurseId = SelectRandomNurseId(decoded1);

    std::vector<int>* s1 = FindRouteByNurseId(decoded1, nurseId);
    std::vector<int>* s2 = FindRouteByNurseId(decoded2, nurseId);

    if (s1 == nullptr || s2 == nullptr) 
    {
        return child;
    }

    std::vector<int> lambda1 = GetDifferentPatients(*s1, *s2);
    std::vector<int> lambda2 = GetDifferentPatients(*s2, *s1);

    RemovePatientsFromDecoded(decoded1, lambda1);
    InsertPatients(decoded1, lambda2);

    child.code = BuildCode(decoded1);
    return child;
}