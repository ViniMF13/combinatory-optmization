#pragma once
#include "Algorithm.hpp"
#include <random>

enum class CoolingSchedule {
    GEOMETRIC,
    LINEAR
};

class SimulatedAnnealing : public Algorithm {
private:
    NeighborhoodType neighborhood;
    CoolingSchedule schedule;
    double initial_temp;
    double final_temp;
    double alpha;
    bool auto_calibrate;
    int window_size;

public:
    SimulatedAnnealing(CoolingSchedule sched = CoolingSchedule::GEOMETRIC,
                       NeighborhoodType neigh = NeighborhoodType::SWAP,
                       double t0 = -1.0, double tf = -1.0, double a = -1.0,
                       int win_size = 500);

    // Construtor de compatibilidade
    SimulatedAnnealing(NeighborhoodType neigh,
                       double t0 = -1.0, double tf = -1.0, double a = -1.0,
                       int win_size = 500)
        : SimulatedAnnealing(CoolingSchedule::GEOMETRIC, neigh, t0, tf, a, win_size) {}

    AlgorithmResult run(const CombinationProblem* prob, int budget, unsigned int seed = 42) override;

    std::string get_name() const override;
    CoolingSchedule get_schedule() const { return schedule; }
};
