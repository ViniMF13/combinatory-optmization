#include "AssetsMaintainance.hpp"
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <numeric>

AssetsMaintainance::AssetsMaintainance(const std::string& filepath, unsigned int seed)
    : rng(seed) {
    std::ifstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Erro ao abrir arquivo: " + filepath);

    int n;
    file >> n;

    double x0, y0;
    file >> x0 >> y0;
    assets.push_back({0, x0, y0, 0.0, 0.0, 0.0}); // Base (índice 0)

    for (int i = 1; i <= n; ++i) {
        double x, y, p, d, w;
        file >> x >> y >> p >> d >> w;
        assets.push_back({i, x, y, p, d, w});
    }
    precompute_distances();
}

void AssetsMaintainance::precompute_distances() {
    int n = assets.size();
    distance_matrix.assign(n, std::vector<int>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double dx = assets[i].x - assets[j].x;
            double dy = assets[i].y - assets[j].y;
            // Arredondamento para o inteiro mais próximo exigido na Equação (1)
            distance_matrix[i][j] = std::round(std::sqrt(dx * dx + dy * dy));
        }
    }
}

double AssetsMaintainance::evaluate(const std::vector<int>& solution) const {
    double current_time = 0.0;
    int prev_asset = 0; // Base
    double total_cost = 0.0;

    for (int asset_id : solution) {
        int travel_time = distance_matrix[prev_asset][asset_id];
        current_time += travel_time + assets[asset_id].p;
        double delay = current_time - assets[asset_id].d;
        if (delay > 0.0) {
            total_cost += assets[asset_id].w * delay;
        }
        prev_asset = asset_id;
    }

    return total_cost;
}

PrefixState AssetsMaintainance::compute_prefix_state(const std::vector<int>& solution) const {
    int n = solution.size();
    PrefixState state;
    state.C.resize(n);
    state.cost.resize(n);

    double current_time = 0.0;
    int prev_asset = 0;
    double current_cost = 0.0;

    for (int k = 0; k < n; ++k) {
        int asset_id = solution[k];
        int travel = distance_matrix[prev_asset][asset_id];
        current_time += travel + assets[asset_id].p;
        double delay = current_time - assets[asset_id].d;
        if (delay > 0.0) {
            current_cost += assets[asset_id].w * delay;
        }
        state.C[k] = current_time;
        state.cost[k] = current_cost;
        prev_asset = asset_id;
    }
    return state;
}

double AssetsMaintainance::evaluate_incremental(const std::vector<int>& current_solution, const PrefixState& state, int i, int j, NeighborhoodType op) const {
    int n = current_solution.size();
    int start = 0;

    if (op == NeighborhoodType::SWAP || op == NeighborhoodType::TWO_OPT) {
        if (i > j) std::swap(i, j);
        start = i;
    } else { // INSERTION
        start = std::min(i, j);
    }

    int prev_asset = (start > 0) ? current_solution[start - 1] : 0;
    double curr_time = (start > 0) ? state.C[start - 1] : 0.0;
    double total_cost = (start > 0) ? state.cost[start - 1] : 0.0;

    for (int k = start; k < n; ++k) {
        int asset_id = 0;
        if (op == NeighborhoodType::SWAP) {
            if (k == i) asset_id = current_solution[j];
            else if (k == j) asset_id = current_solution[i];
            else asset_id = current_solution[k];
        } else if (op == NeighborhoodType::TWO_OPT) {
            if (k >= i && k <= j) asset_id = current_solution[j - (k - i)];
            else asset_id = current_solution[k];
        } else { // INSERTION
            if (i < j) {
                if (k >= i && k < j) asset_id = current_solution[k + 1];
                else if (k == j) asset_id = current_solution[i];
                else asset_id = current_solution[k];
            } else {
                if (k == j) asset_id = current_solution[i];
                else if (k > j && k <= i) asset_id = current_solution[k - 1];
                else asset_id = current_solution[k];
            }
        }

        int travel = distance_matrix[prev_asset][asset_id];
        curr_time += travel + assets[asset_id].p;
        double delay = curr_time - assets[asset_id].d;
        if (delay > 0.0) {
            total_cost += assets[asset_id].w * delay;
        }
        prev_asset = asset_id;
    }

    return total_cost;
}

double AssetsMaintainance::evaluate_incremental(const std::vector<int>& current_solution, double /*current_cost*/, int i, int j, const std::string& move_type) const {
    NeighborhoodType op = NeighborhoodType::SWAP;
    if (move_type == "insertion" || move_type == "INSERTION") op = NeighborhoodType::INSERTION;
    else if (move_type == "two_opt" || move_type == "2-opt" || move_type == "TWO_OPT") op = NeighborhoodType::TWO_OPT;

    PrefixState state = compute_prefix_state(current_solution);
    return evaluate_incremental(current_solution, state, i, j, op);
}

std::vector<int> AssetsMaintainance::random_solution() const {
    int n = get_size();
    std::vector<int> sol(n);
    std::iota(sol.begin(), sol.end(), 1);
    std::shuffle(sol.begin(), sol.end(), rng);
    return sol;
}

std::vector<int> AssetsMaintainance::apply_swap(const std::vector<int>& s, int i, int j) {
    std::vector<int> res = s;
    std::swap(res[i], res[j]);
    return res;
}

std::vector<int> AssetsMaintainance::apply_insertion(const std::vector<int>& s, int i, int j) {
    if (i == j) return s;
    std::vector<int> res = s;
    int val = res[i];
    res.erase(res.begin() + i);
    res.insert(res.begin() + j, val);
    return res;
}

std::vector<int> AssetsMaintainance::apply_two_opt(const std::vector<int>& s, int i, int j) {
    if (i > j) std::swap(i, j);
    std::vector<int> res = s;
    std::reverse(res.begin() + i, res.begin() + j + 1);
    return res;
}

std::vector<int> AssetsMaintainance::apply_move(const std::vector<int>& s, int i, int j, NeighborhoodType op) {
    switch (op) {
        case NeighborhoodType::SWAP:
            return apply_swap(s, i, j);
        case NeighborhoodType::INSERTION:
            return apply_insertion(s, i, j);
        case NeighborhoodType::TWO_OPT:
            return apply_two_opt(s, i, j);
    }
    return s;
}

std::vector<std::vector<int>> AssetsMaintainance::get_successors(const std::vector<int>& s, NeighborhoodType op) const {
    int n = s.size();
    std::vector<std::vector<int>> succs;

    if (op == NeighborhoodType::SWAP) {
        succs.reserve(n * (n - 1) / 2);
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                succs.push_back(apply_swap(s, i, j));
            }
        }
    } else if (op == NeighborhoodType::INSERTION) {
        succs.reserve(n * (n - 1));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j) {
                    succs.push_back(apply_insertion(s, i, j));
                }
            }
        }
    } else if (op == NeighborhoodType::TWO_OPT) {
        succs.reserve(n * (n - 1) / 2);
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                succs.push_back(apply_two_opt(s, i, j));
            }
        }
    }

    return succs;
}

void AssetsMaintainance::for_each_successor(const std::vector<int>& s, NeighborhoodType op, const std::function<bool(const std::vector<int>&)>& callback) const {
    int n = s.size();
    if (op == NeighborhoodType::SWAP) {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (!callback(apply_swap(s, i, j))) return;
            }
        }
    } else if (op == NeighborhoodType::INSERTION) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j) {
                    if (!callback(apply_insertion(s, i, j))) return;
                }
            }
        }
    } else if (op == NeighborhoodType::TWO_OPT) {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (!callback(apply_two_opt(s, i, j))) return;
            }
        }
    }
}

std::vector<int> AssetsMaintainance::random_successor(const std::vector<int>& s, NeighborhoodType op) const {
    int n = s.size();
    std::uniform_int_distribution<int> dist(0, n - 1);
    int i = dist(rng);
    int j = dist(rng);

    while (i == j) {
        j = dist(rng);
    }

    if ((op == NeighborhoodType::SWAP || op == NeighborhoodType::TWO_OPT) && i > j) {
        std::swap(i, j);
    }

    return apply_move(s, i, j, op);
}

std::vector<int> AssetsMaintainance::apply_ox(const std::vector<int>& p1, const std::vector<int>& p2, int c1, int c2) {
    int n = p1.size();
    if (c1 > c2) std::swap(c1, c2);
    if (c1 == c2) {
        c2 = (c1 == n - 1) ? c1 : c1 + 1;
    }

    std::vector<int> child(n, 0);
    std::vector<bool> in_child(n + 1, false);

    for (int k = c1; k <= c2; ++k) {
        child[k] = p1[k];
        in_child[child[k]] = true;
    }

    int child_idx = (c2 + 1) % n;
    for (int step = 0; step < n; ++step) {
        int p2_idx = (c2 + 1 + step) % n;
        int gene = p2[p2_idx];
        if (!in_child[gene]) {
            child[child_idx] = gene;
            in_child[gene] = true;
            child_idx = (child_idx + 1) % n;
        }
    }

    return child;
}

std::vector<int> AssetsMaintainance::apply_pmx(const std::vector<int>& p1, const std::vector<int>& p2, int c1, int c2) {
    int n = p1.size();
    if (c1 > c2) std::swap(c1, c2);
    if (c1 == c2) {
        c2 = (c1 == n - 1) ? c1 : c1 + 1;
    }

    std::vector<int> child(n, 0);
    std::vector<int> p2_to_idx(n + 1, -1);
    for (int i = 0; i < n; ++i) {
        p2_to_idx[p2[i]] = i;
    }

    std::vector<bool> in_child(n + 1, false);

    // Passo 1: Copia segmento p1[c1..c2] para o filho
    for (int i = c1; i <= c2; ++i) {
        child[i] = p1[i];
        in_child[child[i]] = true;
    }

    // Passo 2: Mapeia elementos de p2[c1..c2]
    for (int i = c1; i <= c2; ++i) {
        int val = p2[i];
        if (!in_child[val]) {
            int pos = i;
            while (pos >= c1 && pos <= c2) {
                int mapped_val = p1[pos];
                pos = p2_to_idx[mapped_val];
            }
            child[pos] = val;
            in_child[val] = true;
        }
    }

    // Passo 3: Copia elementos restantes de p2
    for (int i = 0; i < n; ++i) {
        if (child[i] == 0) {
            child[i] = p2[i];
        }
    }

    return child;
}

std::vector<int> AssetsMaintainance::crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, CrossoverType type) const {
    int n = parent1.size();
    std::uniform_int_distribution<int> dist(0, n - 1);
    int c1 = dist(rng);
    int c2 = dist(rng);

    if (type == CrossoverType::PMX) {
        return apply_pmx(parent1, parent2, c1, c2);
    }
    return apply_ox(parent1, parent2, c1, c2);
}

std::vector<int> AssetsMaintainance::mutate(const std::vector<int>& s) const {
    // Swap mutation clássica
    int n = s.size();
    std::uniform_int_distribution<int> dist(0, n - 1);
    int i = dist(rng);
    int j = dist(rng);
    while (i == j) {
        j = dist(rng);
    }
    return apply_swap(s, i, j);
}
