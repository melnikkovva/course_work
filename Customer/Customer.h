#pragma once

class Customer 
{
public:
    Customer(int id, double x, double y,
        int requiredSkill, double serviceTime,
        double appointmentTime, double baseRate
    );
    int GetId() const;
    double GetX() const;
    double GetY() const;
    int GetRequiredSkill() const;
    double GetServiceTime() const;
    double GetAppointmentTime() const;
    double GetBaseRate() const;

private:
    int m_id = 0;
    double m_x = 0;
    double m_y = 0;
    int m_requiredSkill = 0;
    double m_serviceTime = 0;
    double m_appointmentTime = 0;
    double m_baseRate = 0;
};