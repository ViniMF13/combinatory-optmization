#pragma once
#include <vector>
#include <functional>

enum class NeighborhoodType {
    SWAP,
    INSERTION,
    TWO_OPT
};

enum class CrossoverType {
    OX,
    PMX
};

/**
 * @brief Classe abstrata para isolar os algoritmos do domínio do problema, conforme exige a seção 2.1 do enunciado.
 */
class CombinationProblem {
public:
    virtual ~CombinationProblem() = default;
    
    virtual int get_size() const = 0;
    
    // Função de avaliação, sempre de minimização conforme seção 3.
    virtual double evaluate(const std::vector<int>& solution) const = 0;
    double h(const std::vector<int>& solution) const { return evaluate(solution); }
    
    // Gera um ponto de partida com permutação aleatória.
    virtual std::vector<int> random_solution() const = 0;
    std::vector<int> solucao_aleatoria() const { return random_solution(); }
    
    // Operadores de vizinhança conforme seção 2.1.
    virtual std::vector<std::vector<int>> get_successors(const std::vector<int>& s, NeighborhoodType op = NeighborhoodType::SWAP) const = 0;
    std::vector<std::vector<int>> sucessores(const std::vector<int>& s) const {
        return get_successors(s, NeighborhoodType::SWAP);
    }
    
    virtual void for_each_successor(const std::vector<int>& s, NeighborhoodType op, const std::function<bool(const std::vector<int>&)>& callback) const {
        auto succs = get_successors(s, op);
        for (const auto& succ : succs) {
            if (!callback(succ)) break;
        }
    }

    virtual std::vector<int> random_successor(const std::vector<int>& s, NeighborhoodType op = NeighborhoodType::SWAP) const = 0;
    std::vector<int> sucessor_aleatorio(const std::vector<int>& s) const {
        return random_successor(s, NeighborhoodType::SWAP);
    }
    
    // Operadores de recombinação (Order Crossover - OX, Partially Mapped Crossover - PMX)
    virtual std::vector<int> crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, CrossoverType type = CrossoverType::OX) const = 0;
    std::vector<int> cruzamento(const std::vector<int>& s1, const std::vector<int>& s2, CrossoverType type = CrossoverType::OX) const {
        return crossover(s1, s2, type);
    }
    
    // Operador de mutação
    virtual std::vector<int> mutate(const std::vector<int>& s) const = 0;
    std::vector<int> mutacao(const std::vector<int>& s) const {
        return mutate(s);
    }
};
