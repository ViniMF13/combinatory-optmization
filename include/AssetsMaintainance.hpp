#pragma once
#include "CombinationProblem.hpp"
#include <string>
#include <vector>
#include <random>

struct Asset {
    int id;
    double x, y, p, d, w;
};

struct PrefixState {
    std::vector<double> C;
    std::vector<double> cost;
};

class AssetsMaintainance : public CombinationProblem {
private:
    std::vector<Asset> assets;
    std::vector<std::vector<int>> distance_matrix; // Cache de t_ij
    mutable std::mt19937 rng;

    void precompute_distances();

public:
    AssetsMaintainance(const std::string& filepath, unsigned int seed = 42);
    
    int get_size() const override { return static_cast<int>(assets.size()) - 1; }
    int get_distance(int i, int j) const { return distance_matrix[i][j]; }
    const Asset& get_asset(int id) const { return assets[id]; }
    const Asset& get_Asset(int id) const { return assets[id]; }

    // Fitness
    double evaluate(const std::vector<int>& solution) const override;
    
    // Prefix cache para avaliação incremental ultrarrápida
    PrefixState compute_prefix_state(const std::vector<int>& solution) const;
    
    // Avaliação incremental
    double evaluate_incremental(const std::vector<int>& current_solution, double current_cost, int i, int j, const std::string& move_type) const;
    double evaluate_incremental(const std::vector<int>& current_solution, const PrefixState& state, int i, int j, NeighborhoodType op) const;

    // Interface comum (Seção 2.1 e 2.2)
    std::vector<int> random_solution() const override;
    std::vector<std::vector<int>> get_successors(const std::vector<int>& s, NeighborhoodType op = NeighborhoodType::SWAP) const override;
    void for_each_successor(const std::vector<int>& s, NeighborhoodType op, const std::function<bool(const std::vector<int>&)>& callback) const override;
    std::vector<int> random_successor(const std::vector<int>& s, NeighborhoodType op = NeighborhoodType::SWAP) const override;
    
    // Operadores genéticos (OX, PMX e Swap Mutation)
    std::vector<int> crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, CrossoverType type = CrossoverType::OX) const override;
    std::vector<int> mutate(const std::vector<int>& s) const override;

    // Métodos utilitários de aplicação direta de movimentos e cruzamentos
    static std::vector<int> apply_swap(const std::vector<int>& s, int i, int j);
    static std::vector<int> apply_insertion(const std::vector<int>& s, int i, int j);
    static std::vector<int> apply_two_opt(const std::vector<int>& s, int i, int j);
    static std::vector<int> apply_move(const std::vector<int>& s, int i, int j, NeighborhoodType op);
    static std::vector<int> apply_ox(const std::vector<int>& p1, const std::vector<int>& p2, int c1, int c2);
    static std::vector<int> apply_pmx(const std::vector<int>& p1, const std::vector<int>& p2, int c1, int c2);
};
