#include "AssetsMaintainance.hpp"
#include "GreedyDescent.hpp"
#include "SimulatedAnnealing.hpp"
#include "GeneticAlgorithm.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>

struct InstanceConfig {
    std::string name;
    std::string filepath;
    int budget;
    int runs;
};

// =========================================================================
// TAREFA T2: Comparação das Vizinhanças (Swap, Insertion, 2-opt)
// =========================================================================
void run_t2_neighborhood_comparison(const std::vector<InstanceConfig>& instances, bool quick_mode = false) {
    std::cout << "\n========================================================" << std::endl;
    std::cout << ">>> Executando Tarefa T2: Comparação das Vizinhanças <<<" << std::endl;
    std::cout << "========================================================" << std::endl;

    std::ofstream out_csv("comparacao_vizinhancas.csv");
    out_csv << "Instancia,Vizinhanca,Execucao,MelhorCusto,Avaliacoes,TempoMs,PassosMelhoria,Restarts\n";

    std::vector<std::pair<NeighborhoodType, std::string>> vizinhancas = {
        {NeighborhoodType::SWAP, "Swap"},
        {NeighborhoodType::INSERTION, "Insertion"},
        {NeighborhoodType::TWO_OPT, "2-opt"}
    };

    for (const auto& inst : instances) {
        int budget = quick_mode ? std::min(inst.budget, 2000) : inst.budget;
        int runs = quick_mode ? 2 : std::min(inst.runs, 10); // 10 execuções para T2 é suficiente para amostragem sólida

        std::cout << "\n--> Instância: " << inst.name << " (Orçamento: " << budget << ", Execuções: " << runs << ")" << std::endl;
        AssetsMaintainance problem(inst.filepath);

        for (const auto& [neigh, neigh_name] : vizinhancas) {
            std::cout << "  [*] Testando vizinhança: " << neigh_name << " ... " << std::flush;
            GreedyDescent gd(GDVariant::BEST_IMPROVEMENT, neigh);

            double sum_cost = 0.0;
            double sum_time = 0.0;

            for (int r = 1; r <= runs; ++r) {
                unsigned int seed = 1000 * r + 42;
                AlgorithmResult res = gd.run(&problem, budget, seed);

                out_csv << inst.name << ","
                        << neigh_name << ","
                        << r << ","
                        << std::fixed << std::setprecision(2) << res.best_cost << ","
                        << res.total_evaluations << ","
                        << res.elapsed_time_ms << ","
                        << res.improvement_steps << ","
                        << res.restart_count << "\n";
                out_csv.flush();

                sum_cost += res.best_cost;
                sum_time += res.elapsed_time_ms;
            }

            std::cout << "Concluído! Custo Médio = " << (sum_cost / runs)
                      << " | Tempo Médio = " << (sum_time / runs) << " ms" << std::endl;
        }
    }
    std::cout << "\n[OK] Resultados da Tarefa T2 salvos em 'comparacao_vizinhancas.csv'." << std::endl;
}

// =========================================================================
// TAREFA T3: Protocolo Experimental Principal conforme Seção 3 do Enunciado
// =========================================================================
void run_t3_main_experiments(const std::vector<InstanceConfig>& instances, bool quick_mode = false) {
    std::cout << "\n========================================================" << std::endl;
    std::cout << ">>> Executando Tarefa T3: Protocolo Experimental Principal <<<" << std::endl;
    std::cout << "========================================================" << std::endl;

    std::ofstream out_results("resultados_experimentos.csv");
    out_results << "Instancia,Algoritmo,Execucao,MelhorCusto,Avaliacoes,TempoMs,PassosMelhoria,Restarts\n";

    std::ofstream out_conv("curvas_convergencia.csv");
    out_conv << "Instancia,Algoritmo,Execucao,Avaliacoes,MelhorCusto\n";

    std::ofstream out_sa("sa_aceitacao.csv");
    out_sa << "Instancia,Execucao,Avaliacoes,TaxaAceitacaoPiores\n";

    std::ofstream out_ga("ga_metricas.csv");
    out_ga << "Instancia,Execucao,Avaliacoes,MelhorCusto,MediaPopulacao,Diversidade\n";

    for (const auto& inst : instances) {
        int budget = quick_mode ? std::min(inst.budget, 2000) : inst.budget;
        int runs = quick_mode ? 2 : inst.runs;

        std::cout << "\n==================================================================" << std::endl;
        std::cout << ">>> INSTÂNCIA: " << inst.name << " (n=" << (inst.name == "pequena" ? 12 : (inst.name == "media" ? 50 : 150))
                  << ") | Orçamento: " << budget << " | Execuções: " << runs << " <<<" << std::endl;
        std::cout << "==================================================================" << std::endl;

        AssetsMaintainance problem(inst.filepath);

        // Instancia os algoritmos comparados (incluindo variantes de GD, SA e GA)
        std::vector<std::shared_ptr<Algorithm>> algoritmos = {
            std::make_shared<GreedyDescent>(GDVariant::BEST_IMPROVEMENT, NeighborhoodType::SWAP),
            std::make_shared<GreedyDescent>(GDVariant::FIRST_IMPROVEMENT, NeighborhoodType::SWAP),
            std::make_shared<GreedyDescent>(GDVariant::RANDOM_SUCCESSOR, NeighborhoodType::SWAP),
            std::make_shared<SimulatedAnnealing>(CoolingSchedule::GEOMETRIC, NeighborhoodType::SWAP),
            std::make_shared<SimulatedAnnealing>(CoolingSchedule::LINEAR, NeighborhoodType::SWAP),
            std::make_shared<GeneticAlgorithm>(SelectionType::TOURNAMENT, CrossoverType::OX, 50, 0.90, 0.15, 3, true),
            std::make_shared<GeneticAlgorithm>(SelectionType::TOURNAMENT, CrossoverType::PMX, 50, 0.90, 0.15, 3, true),
            std::make_shared<GeneticAlgorithm>(SelectionType::ROULETTE, CrossoverType::OX, 50, 0.90, 0.15, 3, true),
            std::make_shared<GeneticAlgorithm>(SelectionType::ROULETTE, CrossoverType::PMX, 50, 0.90, 0.15, 3, true)
        };

        for (auto& alg : algoritmos) {
            std::string alg_name = alg->get_name();
            std::cout << "\n--> Algoritmo: [" << alg_name << "]" << std::endl;

            double sum_cost = 0.0;
            double min_cost = 1e18;
            double sum_time = 0.0;

            for (int r = 1; r <= runs; ++r) {
                unsigned int seed = 2000 * r + 137;
                AlgorithmResult res = alg->run(&problem, budget, seed);

                sum_cost += res.best_cost;
                sum_time += res.elapsed_time_ms;
                if (res.best_cost < min_cost) min_cost = res.best_cost;

                // Salva linha principal
                out_results << inst.name << ","
                            << alg_name << ","
                            << r << ","
                            << std::fixed << std::setprecision(2) << res.best_cost << ","
                            << res.total_evaluations << ","
                            << res.elapsed_time_ms << ","
                            << res.improvement_steps << ","
                            << res.restart_count << "\n";
                out_results.flush();

                // Salva pontos da curva de convergência
                for (const auto& [ev, cost] : res.convergence_curve) {
                    out_conv << inst.name << ","
                             << alg_name << ","
                             << r << ","
                             << ev << ","
                             << cost << "\n";
                }
                out_conv.flush();

                // Se for SA, salva curva de aceitação de movimentos piores
                if (!res.sa_worse_acceptance_curve.empty()) {
                    for (const auto& [ev, rate] : res.sa_worse_acceptance_curve) {
                        out_sa << inst.name << ","
                               << r << ","
                               << ev << ","
                               << rate << "\n";
                    }
                    out_sa.flush();
                }

                // Se for GA, salva média e diversidade da população
                if (!res.ga_mean_curve.empty()) {
                    size_t count = std::min(res.ga_mean_curve.size(), res.ga_diversity_curve.size());
                    for (size_t k = 0; k < count; ++k) {
                        double best_at_end = res.convergence_curve.empty() ? 0.0 : res.convergence_curve.back().second;
                        out_ga << inst.name << ","
                               << r << ","
                               << res.ga_mean_curve[k].first << ","
                               << best_at_end << ","
                               << res.ga_mean_curve[k].second << ","
                               << res.ga_diversity_curve[k].second << "\n";
                    }
                    out_ga.flush();
                }

                std::cout << "  Execução " << std::setw(2) << r << "/" << runs
                          << " | Custo: " << std::setw(8) << res.best_cost
                          << " | Evals: " << res.total_evaluations
                          << " | Tempo: " << std::fixed << std::setprecision(1) << res.elapsed_time_ms << " ms"
                          << std::endl;
            }

            std::cout << "  >>> Média: " << (sum_cost / runs)
                      << " | Melhor: " << min_cost
                      << " | Tempo Médio: " << (sum_time / runs) << " ms" << std::endl;
        }
    }

    std::cout << "\n[OK] Experimentos T3 finalizados com sucesso!" << std::endl;
    std::cout << "Arquivos CSV gerados:" << std::endl;
    std::cout << " - resultados_experimentos.csv" << std::endl;
    std::cout << " - curvas_convergencia.csv" << std::endl;
    std::cout << " - sa_aceitacao.csv" << std::endl;
    std::cout << " - ga_metricas.csv" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string mode = "--all";
    if (argc > 1) {
        mode = argv[1];
    }

    std::vector<InstanceConfig> instances = {
        {"pequena", "data/manutencao_pequena.txt", 50000, 30},
        {"media",   "data/manutencao_media.txt",   150000, 30},
        {"grande",  "data/manutencao_grande.txt",  300000, 10}
    };

    std::cout << "==========================================================" << std::endl;
    std::cout << "  TP2 - FUNDAMENTOS DE IA - BATERIA DE TESTES      " << std::endl;
    std::cout << "==========================================================" << std::endl;

    if (mode == "--test") {
        std::cout << "Modo rápido (--test) selecionado: validação do pipeline em poucos segundos." << std::endl;
        run_t2_neighborhood_comparison(instances, true);
        run_t3_main_experiments(instances, true);
    } else if (mode == "--t2") {
        run_t2_neighborhood_comparison(instances, false);
    } else if (mode == "--t3") {
        run_t3_main_experiments(instances, false);
    } else { // --all
        run_t2_neighborhood_comparison(instances, false);
        run_t3_main_experiments(instances, false);
    }

    return 0;
}
