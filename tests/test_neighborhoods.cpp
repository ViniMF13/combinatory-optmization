#include <gtest/gtest.h>
#include "AssetsMaintainance.hpp"
#include <numeric>
#include <algorithm>
#include <fstream>
#include <chrono>

static std::string resolve_path_neighborhood(const std::string& rel) {
    std::vector<std::string> prefixes = {"", "../", "../../", "../../../"};
    for (const auto& p : prefixes) {
        std::ifstream f(p + rel);
        if (f.good()) return p + rel;
    }
    return rel;
}

class NeighborhoodTest : public ::testing::Test {
protected:
    AssetsMaintainance* smallProblem = nullptr;
    AssetsMaintainance* mediumProblem = nullptr;

    void SetUp() override {
        smallProblem = new AssetsMaintainance(resolve_path_neighborhood("data/manutencao_pequena.txt"));
        mediumProblem = new AssetsMaintainance(resolve_path_neighborhood("data/manutencao_media.txt"));
    }

    void TearDown() override {
        delete smallProblem;
        delete mediumProblem;
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

TEST_F(NeighborhoodTest, TamanhoDasVizinhançasConformeTeoria) {
    std::vector<int> sol = smallProblem->random_solution();
    int n = smallProblem->get_size(); // 12

    auto swap_succs = smallProblem->get_successors(sol, NeighborhoodType::SWAP);
    EXPECT_EQ(swap_succs.size(), static_cast<size_t>(n * (n - 1) / 2)); // 66

    auto ins_succs = smallProblem->get_successors(sol, NeighborhoodType::INSERTION);
    EXPECT_EQ(ins_succs.size(), static_cast<size_t>(n * (n - 1))); // 132

    auto two_opt_succs = smallProblem->get_successors(sol, NeighborhoodType::TWO_OPT);
    EXPECT_EQ(two_opt_succs.size(), static_cast<size_t>(n * (n - 1) / 2)); // 66

    // Validar todas as permutações geradas
    for (const auto& s : swap_succs) EXPECT_TRUE(is_valid_permutation(s, n));
    for (const auto& s : ins_succs) EXPECT_TRUE(is_valid_permutation(s, n));
    for (const auto& s : two_opt_succs) EXPECT_TRUE(is_valid_permutation(s, n));
}

TEST_F(NeighborhoodTest, AvaliacaoIncrementalConcordaComCompletaEmTodasAsVizinhancas) {
    int n = smallProblem->get_size();
    std::vector<int> sol = smallProblem->random_solution();
    double base_cost = smallProblem->evaluate(sol);
    PrefixState state = smallProblem->compute_prefix_state(sol);

    // Testar todos os movimentos de Swap possíveis
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            std::vector<int> swapped = AssetsMaintainance::apply_swap(sol, i, j);
            double full_eval = smallProblem->evaluate(swapped);
            double inc_eval = smallProblem->evaluate_incremental(sol, state, i, j, NeighborhoodType::SWAP);
            double inc_str = smallProblem->evaluate_incremental(sol, base_cost, i, j, "swap");

            EXPECT_DOUBLE_EQ(inc_eval, full_eval) << "Falha em Swap i=" << i << ", j=" << j;
            EXPECT_DOUBLE_EQ(inc_str, full_eval) << "Falha em Swap (string) i=" << i << ", j=" << j;
        }
    }

    // Testar todos os movimentos de Insertion possíveis
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            std::vector<int> inserted = AssetsMaintainance::apply_insertion(sol, i, j);
            double full_eval = smallProblem->evaluate(inserted);
            double inc_eval = smallProblem->evaluate_incremental(sol, state, i, j, NeighborhoodType::INSERTION);
            double inc_str = smallProblem->evaluate_incremental(sol, base_cost, i, j, "insertion");

            EXPECT_DOUBLE_EQ(inc_eval, full_eval) << "Falha em Insertion i=" << i << ", j=" << j;
            EXPECT_DOUBLE_EQ(inc_str, full_eval) << "Falha em Insertion (string) i=" << i << ", j=" << j;
        }
    }

    // Testar todos os movimentos de 2-opt possíveis
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            std::vector<int> two_opt = AssetsMaintainance::apply_two_opt(sol, i, j);
            double full_eval = smallProblem->evaluate(two_opt);
            double inc_eval = smallProblem->evaluate_incremental(sol, state, i, j, NeighborhoodType::TWO_OPT);
            double inc_str = smallProblem->evaluate_incremental(sol, base_cost, i, j, "two_opt");

            EXPECT_DOUBLE_EQ(inc_eval, full_eval) << "Falha em 2-opt i=" << i << ", j=" << j;
            EXPECT_DOUBLE_EQ(inc_str, full_eval) << "Falha em 2-opt (string) i=" << i << ", j=" << j;
        }
    }
}

TEST_F(NeighborhoodTest, CrossoverEMutacaoGeramPermutacoesValidas) {
    int n = smallProblem->get_size();
    for (int trial = 0; trial < 100; ++trial) {
        std::vector<int> p1 = smallProblem->random_solution();
        std::vector<int> p2 = smallProblem->random_solution();
        
        // Testa Order Crossover (OX)
        std::vector<int> child_ox = smallProblem->crossover(p1, p2, CrossoverType::OX);
        EXPECT_TRUE(is_valid_permutation(child_ox, n));

        // Testa Partially Mapped Crossover (PMX)
        std::vector<int> child_pmx = smallProblem->crossover(p1, p2, CrossoverType::PMX);
        EXPECT_TRUE(is_valid_permutation(child_pmx, n));

        std::vector<int> mut = smallProblem->mutate(p1);
        EXPECT_TRUE(is_valid_permutation(mut, n));
    }
}

TEST_F(NeighborhoodTest, MedicaoGanhoTempoAvaliacaoIncremental) {
    int n = mediumProblem->get_size(); // 50
    std::vector<int> sol = mediumProblem->random_solution();
    PrefixState state = mediumProblem->compute_prefix_state(sol);

    const int iterations = 10000;

    // Benchmark avaliação completa
    auto start_full = std::chrono::high_resolution_clock::now();
    double dummy_full = 0.0;
    for (int k = 0; k < iterations; ++k) {
        int i = k % n;
        int j = (k * 7 + 1) % n;
        if (i == j) j = (j + 1) % n;
        if (i > j) std::swap(i, j);
        std::vector<int> sw = AssetsMaintainance::apply_swap(sol, i, j);
        dummy_full += mediumProblem->evaluate(sw);
    }
    auto end_full = std::chrono::high_resolution_clock::now();
    double time_full = std::chrono::duration<double, std::milli>(end_full - start_full).count();

    // Benchmark avaliação incremental
    auto start_inc = std::chrono::high_resolution_clock::now();
    double dummy_inc = 0.0;
    for (int k = 0; k < iterations; ++k) {
        int i = k % n;
        int j = (k * 7 + 1) % n;
        if (i == j) j = (j + 1) % n;
        if (i > j) std::swap(i, j);
        dummy_inc += mediumProblem->evaluate_incremental(sol, state, i, j, NeighborhoodType::SWAP);
    }
    auto end_inc = std::chrono::high_resolution_clock::now();
    double time_inc = std::chrono::duration<double, std::milli>(end_inc - start_inc).count();

    EXPECT_DOUBLE_EQ(dummy_full, dummy_inc);
    std::cout << "\n==========================================" << std::endl;
    std::cout << "[BENCHMARK AVALIAÇÃO INCREMENTAL - n=50]" << std::endl;
    std::cout << "10.000 avaliações completas:    " << time_full << " ms" << std::endl;
    std::cout << "10.000 avaliações incrementais: " << time_inc << " ms" << std::endl;
    std::cout << "Aceleração (Speedup):           " << (time_full / time_inc) << "x mais rápido" << std::endl;
    std::cout << "==========================================\n" << std::endl;

    EXPECT_GT(time_full, time_inc);
}
