#pragma once

#include "Entities.h"
#include <random>
#include <utility>
#include <vector>

class MOBSO {
public:
    MOBSO(std::vector<Patient> patients, std::vector<CareCenter> centers);
    bool IsDominate(const Solution& a, const Solution& b) const;
    std::vector<Cluster> CreateClusters(std::vector<Solution> population) const;
    Solution Crossover(const Solution& parent1, const Solution& parent2);
    void Mutate(Solution& solution);

private:
    std::vector<Patient> m_patients;
    std::vector<CareCenter> m_centers;
    std::vector<Nurse> m_nurses;
    std::mt19937 m_gen;

    bool HasEnoughSkill(SkillLevel caregiverSkill, SkillLevel requiredSkill) const;
    const Patient* FindPatient(int id) const;
    const Nurse* FindNurse(int id) const;
    bool IsPatientId(int id) const;
    bool IsNurseId(int id) const;
    DecodedSolution SplitByNurse(const std::vector<int>& code) const;
    std::vector<int> BuildCode(const DecodedSolution& decoded) const;
    int SkillValue(SkillLevel skill) const;
    int SelectRandomNurseId(const DecodedSolution& decoded);
    std::vector<int>* FindRouteByNurseId(DecodedSolution& decoded, int nurseId) const;
    std::vector<int> GetDifferentPatients(const std::vector<int>& first, const std::vector<int>& second) const;
    void RemovePatientsFromDecoded(DecodedSolution& decoded, const std::vector<int>& patients) const;
    void InsertPatients(DecodedSolution& decoded, const std::vector<int>& patients);
    bool TryInsertPatient(DecodedSolution& decoded, int patientId);
    bool CanAssignPatientToNurse(int nurseId, int patientId, int currentWorkload) const;
    std::vector<int> FindMutableRoutes(const DecodedSolution& decoded) const;
};
