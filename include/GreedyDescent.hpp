#pragma once
#include "Algorithm.hpp"
#include <string>

enum class GDVariant {
    BEST_IMPROVEMENT,
    FIRST_IMPROVEMENT,
    RANDOM_SUCCESSOR
};

class GreedyDescent : public Algorithm {
private:
    GDVariant variant;
    NeighborhoodType neighborhood;
    int max_stagnation; // Usado na variante RANDOM_SUCCESSOR para detectar ótimo local

public:
    GreedyDescent(GDVariant var = GDVariant::BEST_IMPROVEMENT,
                  NeighborhoodType neigh = NeighborhoodType::SWAP,
                  int max_stag = -1);

    AlgorithmResult run(const CombinationProblem* prob, int budget, unsigned int seed = 42) override;

    std::string get_name() const override;
    GDVariant get_variant() const { return variant; }
};
