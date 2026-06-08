#include "TestLineProblem.h"
#include <vector>
#include "../CareCenter/CareCenter.h"
#include "../CareGiver/CareGiver.h"
#include "../Customer/Customer.h"

Problem CreateLineProblem()
{
    std::vector<CareCenter> centers =
    {
        CareCenter(1, 3, 2, 10)
    };

    std::vector<Caregiver> caregivers =
    {
        Caregiver(11, 1, 1)
    };

    std::vector<Customer> customers =
    {
        Customer(1, 3, 1, 1, 1, 0, 1),
        Customer(2, 5, 1, 1, 2, 2, 1),
        Customer(3, 7, 1, 1, 1, 5, 1),
        Customer(4, 10, 1, 1, 2, 7, 1),
        Customer(5, 13, 1, 1, 1, 10, 1),
        Customer(6, 16, 1, 1, 2, 12, 1),
        Customer(7, 20, 1, 1, 1, 14, 1),
        Customer(8, 23, 1, 1, 2, 16, 1),
        Customer(9, 26, 1, 1, 1, 19, 1),
        Customer(10, 27, 1, 1, 2, 20, 1)
    };

    return Problem(
        std::move(customers),
        std::move(centers),
        std::move(caregivers),
        10,
        1.0
    );
}