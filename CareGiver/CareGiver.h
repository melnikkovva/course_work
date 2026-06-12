#pragma once

class Caregiver 
{
public:
    Caregiver(int id, int centerId, int skill);
    int GetId() const;
    int GetCenterId() const;
    int GetSkill() const;
private:
    int m_id = 0;
    int m_centerId = 0;
    int m_skill = 0;
};