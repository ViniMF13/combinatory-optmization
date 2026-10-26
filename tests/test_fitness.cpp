#include <gtest/gtest.h>
#include "AssetsMaintainance.hpp"
#include <numeric>

#include <fstream>
#include <string>

static std::string resolve_path(const std::string& rel) {
    std::vector<std::string> prefixes = {"", "../", "../../", "../../../"};
    for (const auto& p : prefixes) {
        std::ifstream f(p + rel);
        if (f.good()) return p + rel;
    }
    return rel;
}

class FitnessTest : public ::testing::Test {
protected:
    AssetsMaintainance* prob = nullptr;
    void SetUp() override {
        std::string path = resolve_path("data/manutencao_pequena.txt");
        prob = new AssetsMaintainance(path);
    }
    void TearDown() override { delete prob; }
};

TEST_F(FitnessTest, IdentidadeRetornaCustoCorreto) {
    std::vector<int> identidade(12);
    std::iota(identidade.begin(), identidade.end(), 1);
    
    double h = prob->evaluate(identidade);
    // Verificação de segurança da Seção 1.4 do PDF (h = 4366)
    EXPECT_DOUBLE_EQ(h, 4366.0); 
}

TEST_F(FitnessTest, GulosaRetornaCustoCorreto) {
    // Heurística gulosa citada na Seção 1.4 do PDF (h = 3168)
    std::vector<int> gulosa = {5, 4, 2, 7, 12, 8, 11, 1, 9, 10, 3, 6};
    double h = prob->evaluate(gulosa);
    EXPECT_DOUBLE_EQ(h, 3168.0);
}

TEST_F(FitnessTest, OtimoGlobalRetornaCustoCorreto) {
    // Ótimo global exato da Seção 1.4 do PDF (h* = 1875)
    std::vector<int> optimal = {5, 4, 1, 6, 7, 2, 8, 12, 3, 10, 9, 11};
    double h = prob->evaluate(optimal);
    EXPECT_DOUBLE_EQ(h, 1875.0);
}
