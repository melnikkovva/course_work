#pragma once

class CareCenter 
{
public:
    CareCenter(int id, double x, double y, int capacity);
    int GetId() const;
    double GetX() const;
    double GetY() const;
    int GetCapacity() const;
private:
    int m_id = 0;
    double m_x = 0;
    double m_y = 0;
    int m_capacity = 0;
};