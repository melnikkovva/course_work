#include "ProblemLoader.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../Customer/Customer.h"
#include "../CareCenter/CareCenter.h"
#include "../CareGiver/CareGiver.h"

Problem ProblemLoader::LoadFromFile(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::vector<Customer> customers;
    std::vector<CareCenter> centers;
    std::vector<Caregiver> caregivers;

    int maxWorkload = 0;
    double delayPenalty = 0.0;

    std::string token;
    std::string section;

    while (file >> token)
    {
        if (token == "MAX_WORKLOAD")
        {
            file >> maxWorkload;
        }
        else if (token == "DELAY_PENALTY")
        {
            file >> delayPenalty;
        }
        else if (token == "CENTERS" || token == "CAREGIVERS" || token == "CUSTOMERS")
        {
            section = token;
        }
        else
        {
            if (section == "CENTERS")
            {
                int id = std::stoi(token);
                double x, y;
                int capacity;

                file >> x >> y >> capacity;

                centers.emplace_back(id, x, y, capacity);
            }
            else if (section == "CAREGIVERS")
            {
                int id = std::stoi(token);
                int centerId, skill;

                file >> centerId >> skill;

                caregivers.emplace_back(id, centerId, skill);
            }
            else if (section == "CUSTOMERS")
            {
                int id = std::stoi(token);
                double x, y;
                int requiredSkill;
                double serviceTime, appointmentTime, baseRate;

                file >> x >> y >> requiredSkill
                     >> serviceTime >> appointmentTime >> baseRate;

                customers.emplace_back(id, x, y, requiredSkill,
                                       serviceTime, appointmentTime, baseRate);
            }
        }
    }

    return Problem(customers, centers, caregivers, maxWorkload, delayPenalty);
}