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
    int m_id{};
    double m_x{};
    double m_y{};
    int m_capacity{};
};