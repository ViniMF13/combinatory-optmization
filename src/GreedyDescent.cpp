#include "GreedyDescent.hpp"
#include <chrono>
#include <algorithm>
#include <limits>
#include <iostream>

GreedyDescent::GreedyDescent(GDVariant var, NeighborhoodType neigh, int max_stag)
    : variant(var), neighborhood(neigh), max_stagnation(max_stag) {}

std::string GreedyDescent::get_name() const {
    std::string v_str;
    switch (variant) {
        case GDVariant::BEST_IMPROVEMENT: v_str = "GD_BestImprovement"; break;
        case GDVariant::FIRST_IMPROVEMENT: v_str = "GD_FirstImprovement"; break;
        case GDVariant::RANDOM_SUCCESSOR: v_str = "GD_RandomSuccessor"; break;
    }
    std::string n_str;
    switch (neighborhood) {
        case NeighborhoodType::SWAP: n_str = "_Swap"; break;
        case NeighborhoodType::INSERTION: n_str = "_Insertion"; break;
        case NeighborhoodType::TWO_OPT: n_str = "_2opt"; break;
    }
    return v_str + n_str;
}

AlgorithmResult GreedyDescent::run(const CombinationProblem* prob, int budget, unsigned int seed) {
    (void)seed; // A aleatoriedade vem da interface CombinationProblem (ou mt19937 se necessário)
    auto start_time = std::chrono::high_resolution_clock::now();

    AlgorithmResult result;
    result.algorithm_name = get_name();
    result.total_evaluations = 0;
    result.improvement_steps = 0;
    result.restart_count = 0;

    int n = prob->get_size();
    int stag_limit = max_stagnation;
    if (stag_limit <= 0) {
        if (neighborhood == NeighborhoodType::INSERTION) {
            stag_limit = n * (n - 1);
        } else {
            stag_limit = n * (n - 1) / 2;
        }
    }

    if (budget <= 0) return result;

    // Ponto de partida inicial
    std::vector<int> current_sol = prob->random_solution();
    double current_cost = prob->evaluate(current_sol);
    result.total_evaluations++;

    std::vector<int> best_sol = current_sol;
    double best_cost = current_cost;

    result.convergence_curve.push_back({result.total_evaluations, best_cost});

    auto record_progress = [&](bool force = false) {
        if (force || result.total_evaluations % 100 == 0 || result.total_evaluations == budget) {
            result.convergence_curve.push_back({result.total_evaluations, best_cost});
        }
    };

    if (variant == GDVariant::BEST_IMPROVEMENT) {
        while (result.total_evaluations < budget) {
            std::vector<int> best_neighbor;
            double best_neighbor_cost = std::numeric_limits<double>::infinity();
            bool has_neighbor = false;

            prob->for_each_successor(current_sol, neighborhood, [&](const std::vector<int>& cand) {
                if (result.total_evaluations >= budget) return false;

                double c = prob->evaluate(cand);
                result.total_evaluations++;
                record_progress();

                if (c < best_neighbor_cost) {
                    best_neighbor_cost = c;
                    best_neighbor = cand;
                    has_neighbor = true;
                }
                return true;
            });

            if (has_neighbor && best_neighbor_cost < current_cost) {
                current_sol = best_neighbor;
                current_cost = best_neighbor_cost;
                result.improvement_steps++;

                if (current_cost < best_cost) {
                    best_cost = current_cost;
                    best_sol = current_sol;
                    record_progress(true);
                }
            } else {
                // Atingiu ótimo local -> Random Restart
                result.restart_count++;
                if (result.total_evaluations < budget) {
                    current_sol = prob->random_solution();
                    current_cost = prob->evaluate(current_sol);
                    result.total_evaluations++;
                    record_progress();

                    if (current_cost < best_cost) {
                        best_cost = current_cost;
                        best_sol = current_sol;
                        record_progress(true);
                    }
                }
            }
        }
    } else if (variant == GDVariant::FIRST_IMPROVEMENT) {
        while (result.total_evaluations < budget) {
            bool found_improvement = false;

            prob->for_each_successor(current_sol, neighborhood, [&](const std::vector<int>& cand) {
                if (result.total_evaluations >= budget) return false;

                double c = prob->evaluate(cand);
                result.total_evaluations++;
                record_progress();

                if (c < current_cost) {
                    current_sol = cand;
                    current_cost = c;
                    result.improvement_steps++;
                    found_improvement = true;

                    if (current_cost < best_cost) {
                        best_cost = current_cost;
                        best_sol = current_sol;
                        record_progress(true);
                    }
                    return false; // Aceita o primeiro e reinicia a varredura
                }
                return true;
            });

            if (!found_improvement && result.total_evaluations < budget) {
                // Atingiu ótimo local -> Random Restart
                result.restart_count++;
                current_sol = prob->random_solution();
                current_cost = prob->evaluate(current_sol);
                result.total_evaluations++;
                record_progress();

                if (current_cost < best_cost) {
                    best_cost = current_cost;
                    best_sol = current_sol;
                    record_progress(true);
                }
            }
        }
    } else if (variant == GDVariant::RANDOM_SUCCESSOR) {
        int consecutive_non_improvements = 0;

        while (result.total_evaluations < budget) {
            std::vector<int> cand = prob->random_successor(current_sol, neighborhood);
            double c = prob->evaluate(cand);
            result.total_evaluations++;
            record_progress();

            if (c < current_cost) {
                current_sol = cand;
                current_cost = c;
                result.improvement_steps++;
                consecutive_non_improvements = 0;

                if (current_cost < best_cost) {
                    best_cost = current_cost;
                    best_sol = current_sol;
                    record_progress(true);
                }
            } else {
                consecutive_non_improvements++;
            }

            if (consecutive_non_improvements >= stag_limit) {
                // Estagnação detectada -> Random Restart
                result.restart_count++;
                consecutive_non_improvements = 0;
                if (result.total_evaluations < budget) {
                    current_sol = prob->random_solution();
                    current_cost = prob->evaluate(current_sol);
                    result.total_evaluations++;
                    record_progress();

                    if (current_cost < best_cost) {
                        best_cost = current_cost;
                        best_sol = current_sol;
                        record_progress(true);
                    }
                }
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    result.elapsed_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    result.best_solution = best_sol;
    result.best_cost = best_cost;

    return result;
}
