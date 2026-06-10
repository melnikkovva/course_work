#pragma once
#include <vector>
#include <algorithm>
#include "../CareCenter/CareCenter.h"
#include "../CareGiver/CareGiver.h"
#include "../Customer/Customer.h"

class Problem 
{
public:
    Problem(
        std::vector<Customer> customers,
        std::vector<CareCenter> centers,
        std::vector<Caregiver> caregivers,
        int maxWorkload,
        double delayPenalty
    );

    const std::vector<Customer>& customers() const;
    const std::vector<CareCenter>& centers() const;
    const std::vector<Caregiver>& caregivers() const;

    int customerCount() const;
    int maxWorkload() const;
    double delayPenalty() const;

    const Customer& GetCustomerById(int id) const;
    const CareCenter& GetCenterById(int id) const;
    const Caregiver& GetCaregiverById(int id) const;

    double CountWayCustomerCustomer(int fromCustomerId, int toCustomerId) const;
    double CountWayCenterCustomer(int centerId, int customerId) const;
    void AddCustomer(const Customer& customer);
    void RemoveCustomerById(int id);

    void AddCaregiver(const Caregiver& caregiver);
    void RemoveCaregiverById(int id);

    void AddCenter(const CareCenter& center);
    void RemoveCenterById(int id);

private:
    std::vector<Customer> m_customers;
    std::vector<CareCenter> m_centers;
    std::vector<Caregiver> m_caregivers;

    int m_maxWorkload{};
    double m_delayPenalty{};
};