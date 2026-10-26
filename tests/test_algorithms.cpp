#include <gtest/gtest.h>
#include "AssetsMaintainance.hpp"
#include "GreedyDescent.hpp"
#include "SimulatedAnnealing.hpp"
#include "GeneticAlgorithm.hpp"
#include <fstream>
#include <algorithm>

static std::string resolve_path_alg(const std::string& rel) {
    std::vector<std::string> prefixes = {"", "../", "../../", "../../../"};
    for (const auto& p : prefixes) {
        std::ifstream f(p + rel);
        if (f.good()) return p + rel;
    }
    return rel;
}

class AlgorithmTest : public ::testing::Test {
protected:
    AssetsMaintainance* prob = nullptr;

    void SetUp() override {
        prob = new AssetsMaintainance(resolve_path_alg("data/manutencao_pequena.txt"));
    }

    void TearDown() override {
        delete prob;
    }

    bool is_valid_permutation(const std::vector<int>& perm, int n) {
        if (static_cast<int>(perm.size()) != n) return false;
        std::vector<bool> seen(n + 1, false);
        for (int x : perm) {
            if (x < 1 || x > n || seen[x]) return false;
            seen[x] = true;
        }
        return true;
    }
};

TEST_F(AlgorithmTest, GDBestImprovementRespeitaOrcamentoEProduzSolucaoValida) {
    GreedyDescent gd(GDVariant::BEST_IMPROVEMENT, NeighborhoodType::SWAP);
    const int budget = 500;
    auto res = gd.run(prob, budget, 42);

    EXPECT_LE(res.total_evaluations, budget);
    EXPECT_TRUE(is_valid_permutation(res.best_solution, prob->get_size()));
    EXPECT_DOUBLE_EQ(res.best_cost, prob->evaluate(res.best_solution));
    EXPECT_GT(res.improvement_steps, 0);
}

TEST_F(AlgorithmTest, GDFirstImprovementRespeitaOrcamentoEProduzSolucaoValida) {
    GreedyDescent gd(GDVariant::FIRST_IMPROVEMENT, NeighborhoodType::SWAP);
    const int budget = 500;
    auto res = gd.run(prob, budget, 42);

    EXPECT_LE(res.total_evaluations, budget);
    EXPECT_TRUE(is_valid_permutation(res.best_solution, prob->get_size()));
    EXPECT_DOUBLE_EQ(res.best_cost, prob->evaluate(res.best_solution));
    EXPECT_GT(res.improvement_steps, 0);
}

TEST_F(AlgorithmTest, GDRandomSuccessorRespeitaOrcamentoEProduzSolucaoValida) {
    GreedyDescent gd(GDVariant::RANDOM_SUCCESSOR, NeighborhoodType::SWAP);
    const int budget = 500;
    auto res = gd.run(prob, budget, 42);

    EXPECT_LE(res.total_evaluations, budget);
    EXPECT_TRUE(is_valid_permutation(res.best_solution, prob->get_size()));
    EXPECT_DOUBLE_EQ(res.best_cost, prob->evaluate(res.best_solution));
    EXPECT_GT(res.improvement_steps, 0);
}

TEST_F(AlgorithmTest, SimulatedAnnealingGeometricRespeitaOrcamentoERegistraAceitacao) {
    SimulatedAnnealing sa(CoolingSchedule::GEOMETRIC, NeighborhoodType::SWAP, -1.0, -1.0, -1.0, 50);
    const int budget = 1000;
    auto res = sa.run(prob, budget, 42);

    EXPECT_LE(res.total_evaluations, budget);
    EXPECT_TRUE(is_valid_permutation(res.best_solution, prob->get_size()));
    EXPECT_DOUBLE_EQ(res.best_cost, prob->evaluate(res.best_solution));
    EXPECT_FALSE(res.sa_worse_acceptance_curve.empty());
}

TEST_F(AlgorithmTest, SimulatedAnnealingLinearRespeitaOrcamentoERegistraAceitacao) {
    SimulatedAnnealing sa(CoolingSchedule::LINEAR, NeighborhoodType::SWAP, -1.0, -1.0, -1.0, 50);
    const int budget = 1000;
    auto res = sa.run(prob, budget, 42);

    EXPECT_LE(res.total_evaluations, budget);
    EXPECT_TRUE(is_valid_permutation(res.best_solution, prob->get_size()));
    EXPECT_DOUBLE_EQ(res.best_cost, prob->evaluate(res.best_solution));
    EXPECT_FALSE(res.sa_worse_acceptance_curve.empty());
}

TEST_F(AlgorithmTest, GeneticAlgorithmTournamentOXRespeitaOrcamentoERegistraDiversidade) {
    GeneticAlgorithm ga(SelectionType::TOURNAMENT, CrossoverType::OX, 20, 0.9, 0.2, 3, true);
    const int budget = 500;
    auto res = ga.run(prob, budget, 42);

    EXPECT_LE(res.total_evaluations, budget);
    EXPECT_TRUE(is_valid_permutation(res.best_solution, prob->get_size()));
    EXPECT_DOUBLE_EQ(res.best_cost, prob->evaluate(res.best_solution));
    EXPECT_FALSE(res.ga_mean_curve.empty());
    EXPECT_FALSE(res.ga_diversity_curve.empty());
}

TEST_F(AlgorithmTest, GeneticAlgorithmRoulettePMXRespeitaOrcamentoERegistraDiversidade) {
    GeneticAlgorithm ga(SelectionType::ROULETTE, CrossoverType::PMX, 20, 0.9, 0.2, 3, true);
    const int budget = 500;
    auto res = ga.run(prob, budget, 42);

    EXPECT_LE(res.total_evaluations, budget);
    EXPECT_TRUE(is_valid_permutation(res.best_solution, prob->get_size()));
    EXPECT_DOUBLE_EQ(res.best_cost, prob->evaluate(res.best_solution));
    EXPECT_FALSE(res.ga_mean_curve.empty());
    EXPECT_FALSE(res.ga_diversity_curve.empty());
}
