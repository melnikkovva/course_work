#pragma once

#include <vector>

class Route 
{
public:
    explicit Route(int caregiverId);
    Route(int caregiverId, std::vector<int> customers);

    int GetCaregiverId() const;
    const std::vector<int>& GetCustomers() const;
    std::vector<int>& GetCustomers();
    bool IsEmpty() const;
    int GetSize() const;
private:
    int m_caregiverId{};
    std::vector<int> m_customers;
};