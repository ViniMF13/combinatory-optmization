#pragma once
#include "Algorithm.hpp"
#include <random>

enum class SelectionType {
    TOURNAMENT,
    ROULETTE
};

class GeneticAlgorithm : public Algorithm {
private:
    SelectionType selection_type;
    CrossoverType crossover_type;
    int population_size;
    double crossover_prob;
    double mutation_prob;
    int tournament_size;
    bool elitism;

public:
    GeneticAlgorithm(SelectionType sel = SelectionType::TOURNAMENT,
                     CrossoverType cross = CrossoverType::OX,
                     int pop_size = 50,
                     double pc = 0.90,
                     double pm = 0.15,
                     int tour_size = 3,
                     bool elite = true);

    // Construtor de compatibilidade para código legado
    GeneticAlgorithm(int pop_size,
                     double pc = 0.90,
                     double pm = 0.15,
                     int tour_size = 3,
                     bool elite = true)
        : GeneticAlgorithm(SelectionType::TOURNAMENT, CrossoverType::OX, pop_size, pc, pm, tour_size, elite) {}

    AlgorithmResult run(const CombinationProblem* prob, int budget, unsigned int seed = 42) override;

    std::string get_name() const override;
    SelectionType get_selection_type() const { return selection_type; }
    CrossoverType get_crossover_type() const { return crossover_type; }
};
