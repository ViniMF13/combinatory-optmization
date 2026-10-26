#include "GeneticAlgorithm.hpp"
#include <algorithm>
#include <chrono>
#include <set>
#include <numeric>
#include <limits>

GeneticAlgorithm::GeneticAlgorithm(SelectionType sel, CrossoverType cross, int pop_size,
                                   double pc, double pm, int tour_size, bool elite)
    : selection_type(sel), crossover_type(cross), population_size(pop_size),
      crossover_prob(pc), mutation_prob(pm), tournament_size(tour_size), elitism(elite) {}

std::string GeneticAlgorithm::get_name() const {
    std::string sel_str = (selection_type == SelectionType::TOURNAMENT) ? "Tour" : "Roulette";
    std::string cross_str = (crossover_type == CrossoverType::OX) ? "OX" : "PMX";
    return "GA_" + sel_str + "_" + cross_str;
}

AlgorithmResult GeneticAlgorithm::run(const CombinationProblem* prob, int budget, unsigned int seed) {
    auto start_time = std::chrono::high_resolution_clock::now();
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist_01(0.0, 1.0);
    std::uniform_int_distribution<int> dist_pop(0, population_size - 1);

    AlgorithmResult result;
    result.algorithm_name = get_name();
    result.total_evaluations = 0;

    if (budget <= 0) return result;

    struct Individual {
        std::vector<int> genes;
        double cost;
    };

    std::vector<Individual> population;
    population.reserve(population_size);

    std::vector<int> global_best_sol;
    double global_best_cost = std::numeric_limits<double>::infinity();

    // 1. Inicialização da população
    int init_count = std::min(population_size, budget);
    for (int i = 0; i < init_count; ++i) {
        Individual ind;
        ind.genes = prob->random_solution();
        ind.cost = prob->evaluate(ind.genes);
        result.total_evaluations++;

        if (ind.cost < global_best_cost) {
            global_best_cost = ind.cost;
            global_best_sol = ind.genes;
        }
        population.push_back(ind);
    }

    auto record_generation_metrics = [&]() {
        if (population.empty()) return;
        double sum_cost = 0.0;
        std::set<std::vector<int>> unique_chromosomes;

        for (const auto& ind : population) {
            sum_cost += ind.cost;
            unique_chromosomes.insert(ind.genes);
        }

        double mean_cost = sum_cost / population.size();
        double diversity = static_cast<double>(unique_chromosomes.size()) / population.size();

        result.convergence_curve.push_back({result.total_evaluations, global_best_cost});
        result.ga_mean_curve.push_back({result.total_evaluations, mean_cost});
        result.ga_diversity_curve.push_back({result.total_evaluations, diversity});
    };

    record_generation_metrics();

    // Funções de Seleção
    auto tournament_select = [&]() -> const Individual& {
        int best_idx = dist_pop(rng);
        for (int k = 1; k < tournament_size; ++k) {
            int candidate_idx = dist_pop(rng);
            if (population[candidate_idx].cost < population[best_idx].cost) {
                best_idx = candidate_idx;
            }
        }
        return population[best_idx];
    };

    std::vector<double> roulette_cum(population_size, 0.0);
    auto prepare_roulette = [&]() {
        double max_cost = -1.0;
        double min_cost = std::numeric_limits<double>::infinity();
        for (const auto& ind : population) {
            if (ind.cost > max_cost) max_cost = ind.cost;
            if (ind.cost < min_cost) min_cost = ind.cost;
        }

        double delta = 0.05 * (max_cost - min_cost + 1.0);
        double acc = 0.0;
        for (size_t i = 0; i < population.size(); ++i) {
            double fitness = (max_cost - population[i].cost) + delta;
            acc += fitness;
            roulette_cum[i] = acc;
        }
    };

    auto roulette_select = [&]() -> const Individual& {
        double total_fit = roulette_cum.back();
        std::uniform_real_distribution<double> dist_fit(0.0, total_fit);
        double r = dist_fit(rng);
        auto it = std::upper_bound(roulette_cum.begin(), roulette_cum.end(), r);
        size_t idx = std::distance(roulette_cum.begin(), it);
        if (idx >= population.size()) idx = population.size() - 1;
        return population[idx];
    };

    // 2. Ciclo de Gerações
    while (result.total_evaluations < budget) {
        if (selection_type == SelectionType::ROULETTE) {
            roulette_cum.resize(population.size());
            prepare_roulette();
        }

        std::vector<Individual> next_generation;
        next_generation.reserve(population_size);

        // Elitismo: preserva o melhor indivíduo da geração anterior
        if (elitism && !population.empty()) {
            auto best_it = std::min_element(population.begin(), population.end(),
                [](const Individual& a, const Individual& b) { return a.cost < b.cost; });
            next_generation.push_back(*best_it);
        }

        while (static_cast<int>(next_generation.size()) < population_size && result.total_evaluations < budget) {
            const Individual& parent1 = (selection_type == SelectionType::TOURNAMENT) ? tournament_select() : roulette_select();
            const Individual& parent2 = (selection_type == SelectionType::TOURNAMENT) ? tournament_select() : roulette_select();

            Individual child;
            if (dist_01(rng) < crossover_prob) {
                child.genes = prob->crossover(parent1.genes, parent2.genes, crossover_type);
            } else {
                child.genes = parent1.genes;
            }

            if (dist_01(rng) < mutation_prob) {
                child.genes = prob->mutate(child.genes);
            }

            child.cost = prob->evaluate(child.genes);
            result.total_evaluations++;

            if (child.cost < global_best_cost) {
                global_best_cost = child.cost;
                global_best_sol = child.genes;
            }

            next_generation.push_back(child);
        }

        population = std::move(next_generation);
        dist_pop = std::uniform_int_distribution<int>(0, population.size() - 1);
        record_generation_metrics();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    result.elapsed_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    result.best_solution = global_best_sol;
    result.best_cost = global_best_cost;

    return result;
}
