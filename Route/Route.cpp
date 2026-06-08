#include "Route.h"

Route::Route(int caregiverId) : m_caregiverId(caregiverId) {}

Route::Route(int caregiverId, std::vector<int> customers)
    : m_caregiverId(caregiverId), m_customers(std::move(customers)) {}

int Route::GetCaregiverId() const 
{ 
    return m_caregiverId; 
}

std::vector<int>& Route::GetCustomers() 
{ 
    return m_customers; 
}

const std::vector<int>& Route::GetCustomers() const 
{ 
    return m_customers; 
}

bool Route::IsEmpty() const 
{ 
    return m_customers.empty(); 
}

int Route::GetSize() const 
{ 
    return static_cast<int>(m_customers.size()); 
}