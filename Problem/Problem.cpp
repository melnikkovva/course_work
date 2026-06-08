#include "Problem.h"

#include <cmath>
#include <stdexcept>

Problem::Problem(
    std::vector<Customer> customers,
    std::vector<CareCenter> centers,
    std::vector<Caregiver> caregivers,
    int maxWorkload,
    double delayPenalty)
    : m_customers(std::move(customers)),
      m_centers(std::move(centers)),
      m_caregivers(std::move(caregivers)),
      m_maxWorkload(maxWorkload),
      m_delayPenalty(delayPenalty) {}

const std::vector<Customer>& Problem::customers() const { return m_customers; }
const std::vector<CareCenter>& Problem::centers() const { return m_centers; }
const std::vector<Caregiver>& Problem::caregivers() const { return m_caregivers; }

int Problem::customerCount() const { return static_cast<int>(m_customers.size()); }
int Problem::maxWorkload() const { return m_maxWorkload; }
double Problem::delayPenalty() const { return m_delayPenalty; }

const Customer& Problem::GetCustomerById(int id) const 
{
    if (id <= 0 || id > static_cast<int>(m_customers.size())) 
    {
        throw std::out_of_range("Customer id is out of range");
    }

    return m_customers[id - 1];
}

const CareCenter& Problem::GetCenterById(int id) const 
{
    for (const auto& center : m_centers) 
    {
        if (center.GetId() == id) return center;
    }

    throw std::out_of_range("Care center id is out of range");
}

const Caregiver& Problem::GetCaregiverById(int id) const 
{
    for (const auto& caregiver : m_caregivers) 
    {
        if (caregiver.GetId() == id) return caregiver;
    }

    throw std::out_of_range("Caregiver id is out of range");
}

double Problem::CountWayCustomerCustomer(int fromCustomerId, int toCustomerId) const 
{
    const auto& from = GetCustomerById(fromCustomerId);
    const auto& to = GetCustomerById(toCustomerId);

    return std::hypot(from.GetX() - to.GetX(), from.GetY() - to.GetY());
}

double Problem::CountWayCenterCustomer(int centerId, int customerId) const 
{
    const auto& center = GetCenterById(centerId);
    const auto& customer = GetCustomerById(customerId);

    return std::hypot(center.GetX() - customer.GetX(), center.GetY() - customer.GetY());
}

void Problem::AddCustomer(const Customer& customer)
{
    m_customers.push_back(customer);
}

void Problem::RemoveCustomerById(int id)
{
    m_customers.erase(
        std::remove_if(
            m_customers.begin(),
            m_customers.end(),
            [id](const Customer& c)
            {
                return c.GetId() == id;
            }
        ),
        m_customers.end()
    );
}

void Problem::AddCaregiver(const Caregiver& caregiver)
{
    m_caregivers.push_back(caregiver);
}

void Problem::RemoveCaregiverById(int id)
{
    m_caregivers.erase(
        std::remove_if(
            m_caregivers.begin(),
            m_caregivers.end(),
            [id](const Caregiver& c)
            {
                return c.GetId() == id;
            }
        ),
        m_caregivers.end()
    );
}

void Problem::AddCenter(const CareCenter& center)
{
    m_centers.push_back(center);
}

void Problem::RemoveCenterById(int id)
{
    m_centers.erase(
        std::remove_if(
            m_centers.begin(),
            m_centers.end(),
            [id](const CareCenter& c)
            {
                return c.GetId() == id;
            }
        ),
        m_centers.end()
    );
}