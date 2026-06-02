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
    int m_id{};
    double m_x{};
    double m_y{};
    int m_requiredSkill{};
    double m_serviceTime{};
    double m_appointmentTime{};
    double m_baseRate{};
};