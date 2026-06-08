#include "TestBigProblem.h"
#include <vector>
#include "../CareCenter/CareCenter.h"
#include "../CareGiver/CareGiver.h"
#include "../Customer/Customer.h"

Problem CreateBigProblem()
{
    std::vector<CareCenter> centers =
    {
        CareCenter(1, 5, 5, 3),
        CareCenter(2, 22, 5, 3),
        CareCenter(3, 13, 17, 3)
    };

    std::vector<Caregiver> caregivers =
    {
        Caregiver(10, 1, 1),
        Caregiver(11, 1, 2),
        Caregiver(12, 1, 3),

        Caregiver(13, 2, 1),
        Caregiver(14, 2, 2),
        Caregiver(15, 2, 3),

        Caregiver(16, 3, 1),
        Caregiver(17, 3, 2),
        Caregiver(18, 3, 3)
    };

    std::vector<Customer> customers =
    {
        Customer(1, 2, 5, 1, 15, 1, 10),
        Customer(2, 5, 2, 2, 17, 2, 15),
        Customer(3, 6, 7, 3, 18, 1, 18),

        Customer(4, 18, 4, 1, 19, 3, 10),
        Customer(5, 22, 1, 2, 20, 4, 15),
        Customer(6, 25, 7, 3, 15, 1, 18),

        Customer(7, 10, 18, 1, 17, 3, 10),
        Customer(8, 13, 19, 2, 18, 1, 15),
        Customer(9, 15, 16, 3, 19, 8, 18)
    };

    return Problem(std::move(customers), std::move(centers),
        std::move(caregivers), 5, 1.0);
}
