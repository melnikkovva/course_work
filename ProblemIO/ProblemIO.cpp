#include "ProblemIO.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../Customer/Customer.h"
#include "../CareCenter/CareCenter.h"
#include "../CareGiver/CareGiver.h"
#include "../Decoder/Decoder.h"

Problem ProblemIO::LoadFromFile(const std::string& filename)
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

void ProblemIO::SaveSolution(
    const Problem& problem,
    const Solution& solution,
    const std::string& filename)
{
    std::ofstream out(filename);

    if (!out)
    {
        throw std::runtime_error(
            "Cannot create output file");
    }

    out << "Solution\n\n";

    out << "Service cost = "
        << solution.GetObjectives().serviceCost
        << "\n";

    out << "Delay cost = "
        << solution.GetObjectives().delayCost
        << "\n\n";

    Decoder decoder(problem);

    std::vector<Route> routes =
        decoder.Decode(solution);

    for (const Route& route : routes)
    {
        out << "Caregiver "
            << route.GetCaregiverId()
            << "\n";

        out << "Route: ";

        for (int customer :
             route.GetCustomers())
        {
            out << customer << " ";
        }

        out << "\n\n";
    }
}