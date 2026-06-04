#include "Customer.h"

Customer::Customer(int id, double x, double y, int requiredSkill, 
  double serviceTime, double appointmentTime, double baseRate)
  : m_id(id),m_x(x), m_y(y), m_requiredSkill(requiredSkill),
    m_serviceTime(serviceTime), m_appointmentTime(appointmentTime),
    m_baseRate(baseRate) {}

int Customer::GetId() const { return m_id; }
double Customer::GetX() const { return m_x; }
double Customer::GetY() const { return m_y; }
int Customer::GetRequiredSkill() const { return m_requiredSkill; }
double Customer::GetServiceTime() const { return m_serviceTime; }
double Customer::GetAppointmentTime() const { return m_appointmentTime; }
double Customer::GetBaseRate() const { return m_baseRate; }