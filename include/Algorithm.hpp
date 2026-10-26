#pragma once
#include "CombinationProblem.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <utility>

struct AlgorithmResult {
    std::string algorithm_name;
    std::string instance_name;
    int run_id = 0;
    std::vector<int> best_solution;
    double best_cost = 0.0;
    int total_evaluations = 0;
    double elapsed_time_ms = 0.0;
    
    // Curva de convergência: pares (avaliações, melhor_h)
    std::vector<std::pair<int, double>> convergence_curve; 

    // Específico para o Simulated Annealing: taxa de aceitação de movimentos piores ao longo do tempo (eval, rate)
    std::vector<std::pair<int, double>> sa_worse_acceptance_curve;

    // Específico para Genetic Algorithm: média da população e diversidade (eval, valor)
    std::vector<std::pair<int, double>> ga_mean_curve;
    std::vector<std::pair<int, double>> ga_diversity_curve;

    // Específico para Greedy Descent: passos de melhoria e número de restarts
    int improvement_steps = 0;
    int restart_count = 0;
};

/**
 * @brief Interface base para os algoritmos de busca (Seção 2.1).
 */
class Algorithm {
public:
    virtual ~Algorithm() = default;

    /**
     * @brief Executa o algoritmo sob um orçamento estrito de avaliações.
     * @param prob Ponteiro para a interface comum do problema.
     * @param budget Orçamento máximo de avaliações da função objetivo h.
     * @param seed Semente para reprodutibilidade.
     * @return Estrutura AlgorithmResult com solução, métricas e curvas.
     */
    virtual AlgorithmResult run(const CombinationProblem* prob, int budget, unsigned int seed = 42) = 0;

    virtual std::string get_name() const = 0;
};
