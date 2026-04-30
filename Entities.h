#include <vector>
#include <string>

using Cluster = std::vector<Solution>;
using DecodedSolution = std::vector<std::pair<int, std::vector<int>>>;

struct Point 
{
    double x;
    double y;
};

enum class SkillLevel { Q1, Q2, Q3};

struct Patient 
{
    int id;
    Point location;
    double serviceTime; 
    double deadline; 
    double price; 
    SkillLevel requiredSkill; 
};

struct Nurse 
{
    int id;
    int centerId;
    SkillLevel skill; 
    int maxWorkload;
};

struct CareCenter 
{
    int id;
    Point location;
    int maxCapacity; 
    std::vector<Nurse> nurses;
};

struct Costs 
{
    double serviceCost = 0.0;
    double delayCost = 0.0;
};

struct Solution 
{
    std::vector<int> code;
    Costs costs;
    int rank = 0;
    double crowdingDistance = 0.0;
};