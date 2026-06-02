#pragma once

class Caregiver 
{
public:
    Caregiver(int id, int centerId, int skill);
    int GetId() const;
    int GetCenterId() const;
    int GetSkill() const;
private:
    int m_id{};
    int m_centerId{};
    int m_skill{};
};