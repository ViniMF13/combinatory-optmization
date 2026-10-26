#include "SimulatedAnnealing.hpp"
#include <cmath>
#include <chrono>
#include <iostream>

SimulatedAnnealing::SimulatedAnnealing(CoolingSchedule sched, NeighborhoodType neigh, double t0, double tf, double a, int win_size)
    : neighborhood(neigh), schedule(sched), initial_temp(t0), final_temp(tf), alpha(a),
      auto_calibrate(t0 <= 0.0), window_size(win_size) {}

std::string SimulatedAnnealing::get_name() const {
    std::string s_str = (schedule == CoolingSchedule::GEOMETRIC) ? "SA_Geometric" : "SA_Linear";
    std::string n_str;
    switch (neighborhood) {
        case NeighborhoodType::SWAP: n_str = "_Swap"; break;
        case NeighborhoodType::INSERTION: n_str = "_Insertion"; break;
        case NeighborhoodType::TWO_OPT: n_str = "_2opt"; break;
    }
    return s_str + n_str;
}

AlgorithmResult SimulatedAnnealing::run(const CombinationProblem* prob, int budget, unsigned int seed) {
    auto start_time = std::chrono::high_resolution_clock::now();
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist_01(0.0, 1.0);

    AlgorithmResult result;
    result.algorithm_name = get_name();
    result.total_evaluations = 0;

    if (budget <= 0) return result;

    std::vector<int> current_sol = prob->random_solution();
    double current_cost = prob->evaluate(current_sol);
    result.total_evaluations++;

    std::vector<int> best_sol = current_sol;
    double best_cost = current_cost;
    result.convergence_curve.push_back({result.total_evaluations, best_cost});

    // Calibração de T0, Tf e alpha conforme Seção 2.4 do PDF
    double T0 = initial_temp;
    double Tf = final_temp;
    double cooling_rate = alpha;

    if (auto_calibrate) {
        // Estima a magnitude típica de delta_h de movimentos de piora
        double sum_delta = 0.0;
        int delta_count = 0;
        for (int sample = 0; sample < 30 && result.total_evaluations < budget; ++sample) {
            std::vector<int> cand = prob->random_successor(current_sol, neighborhood);
            double cand_c = prob->evaluate(cand);
            result.total_evaluations++;
            if (cand_c > current_cost) {
                sum_delta += (cand_c - current_cost);
                delta_count++;
            }
        }

        double typical_delta = (delta_count > 0) ? (sum_delta / delta_count) : 500.0;
        // P0 ~ 0.80 (quase random walk), Pf ~ 0.001 (quase greedy descent)
        T0 = -typical_delta / std::log(0.80);
        Tf = -typical_delta / std::log(0.001);
        if (Tf >= T0) Tf = T0 * 0.001;

        int remaining_steps = std::max(1, budget - result.total_evaluations);
        cooling_rate = std::pow(Tf / T0, 1.0 / remaining_steps);
    }

    double T = T0;
    int window_worse_proposed = 0;
    int window_worse_accepted = 0;

    auto record_progress = [&](bool force = false) {
        if (force || result.total_evaluations % 100 == 0 || result.total_evaluations == budget) {
            result.convergence_curve.push_back({result.total_evaluations, best_cost});
        }
    };

    while (result.total_evaluations < budget) {
        std::vector<int> cand = prob->random_successor(current_sol, neighborhood);
        double cand_cost = prob->evaluate(cand);
        result.total_evaluations++;

        double delta = cand_cost - current_cost;

        if (delta <= 0.0) {
            // Movimento de melhora ou neutro: aceita sempre
            current_sol = cand;
            current_cost = cand_cost;
            if (current_cost < best_cost) {
                best_cost = current_cost;
                best_sol = current_sol;
                record_progress(true);
            }
        } else {
            // Movimento pior: aceita com probabilidade de Gibbs
            window_worse_proposed++;
            double prob_accept = std::exp(-delta / T);
            if (dist_01(rng) < prob_accept) {
                current_sol = cand;
                current_cost = cand_cost;
                window_worse_accepted++;
            }
        }

        // Resfriamento conforme a variante selecionada
        if (schedule == CoolingSchedule::GEOMETRIC) {
            T = std::max(Tf, T * cooling_rate);
        } else { // LINEAR
            double fraction = static_cast<double>(result.total_evaluations) / budget;
            T = std::max(Tf, T0 - (T0 - Tf) * fraction);
        }

        // Registro da taxa de aceitação de movimentos piores
        if (result.total_evaluations % window_size == 0 || result.total_evaluations == budget) {
            double rate = (window_worse_proposed > 0)
                              ? static_cast<double>(window_worse_accepted) / window_worse_proposed
                              : 0.0;
            result.sa_worse_acceptance_curve.push_back({result.total_evaluations, rate});
            window_worse_proposed = 0;
            window_worse_accepted = 0;
        }

        record_progress();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    result.elapsed_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    result.best_solution = best_sol;
    result.best_cost = best_cost;

    return result;
}
