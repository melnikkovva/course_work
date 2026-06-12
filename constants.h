#include <cstdint>

static constexpr int POPULATION_SIZE = 300;
static constexpr int MAX_EVALUATIONS = 900;
static constexpr double PG = 0.6;
static constexpr double PO = 0.5;
static constexpr double PT = 0.1;
static constexpr std::uint32_t RANDOM_SEED = 42;
static constexpr double INFEASIBLE_PENALTY = 1e9;