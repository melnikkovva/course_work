#include "Caregiver.h"

Caregiver::Caregiver(int id, int centerId, int skill)
    : m_id(id), m_centerId(centerId), m_skill(skill) {}

int Caregiver::GetId() const { return m_id; }
int Caregiver::GetCenterId() const { return m_centerId; }
int Caregiver::GetSkill() const { return m_skill; }