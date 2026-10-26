# TP2 - Fundamentos de Inteligência Artificial (UFMG)
## Comparação de Busca Local e Algoritmo Genético em Otimização Combinatória

Este repositório contém a implementação completa, testada e instrumentada para resolução do **Problema de Sequenciamento de Manutenção de Ativos**, comparando:
1. **Greedy Descent com Random Restart** (3 variantes: *Best Improvement*, *First Improvement* e *Random Successor*);
2. **Simulated Annealing** (resfriamento geométrico $T \leftarrow \alpha T$, distribuição de Gibbs e registro da taxa de aceitação de movimentos piores);
3. **Genetic Algorithm** (Order Crossover - OX, Swap Mutation, seleção por torneio, elitismo e monitoramento de diversidade e média populacional).

---

## 1. Requisitos de Ambiente

- **C++:** Compilador compatível com C++17 (`clang++` ou `g++`) e `cmake` >= 3.14.
- **Python:** Python 3.9+ com pacotes listados em `requirements.txt`.

Instalação das dependências Python:
```bash
pip install -r requirements.txt
```

---

## 2. Compilação e Testes Unitários (TDD)

```bash
mkdir -p build && cd build
cmake ..
make
# Executar a bateria de testes unitários (12 testes cobrindo fitness, vizinhanças e algoritmos)
./tests/test_suite
cd ..
```

---

## 3. Como Executar os Experimentos

### 3.1 Validação Rápida do Pipeline (Smoke Test - ~5 segundos)
Executa 2 repetições com orçamento reduzido para validar a geração de todos os CSVs:
```bash
./build/src/tp2_main --test
```

### 3.2 Tarefa T2: Comparação das Vizinhanças (Swap vs Insertion vs 2-opt)
Executa a comparação dos 3 operadores de vizinhança sob Greedy Descent:
```bash
./build/src/tp2_main --t2
```
*Gera o arquivo `comparacao_vizinhancas.csv`.*

### 3.3 Tarefa T3: Protocolo Experimental Principal Completo
Executa o protocolo estrito da Seção 3 do enunciado:
- **Pequena ($n=12$):** 50.000 avaliações, 30 execuções independentes;
- **Média ($n=50$):** 150.000 avaliações, 30 execuções independentes;
- **Grande ($n=150$):** 300.000 avaliações, 10 execuções independentes.

```bash
./build/src/tp2_main --t3
```
*Gera os arquivos `resultados_experimentos.csv`, `curvas_convergencia.csv`, `sa_aceitacao.csv` e `ga_metricas.csv`.*

### 3.4 Bateria Completa (T2 + T3)
```bash
./build/src/tp2_main --all
```

---

## 4. Geração Automática das Tabelas e Figuras do Relatório

Após a conclusão das execuções, execute o script de análise estatística:
```bash
python3 scripts/analyze.py
```

O script gera automaticamente:
1. **Tabela Principal** (`tabela_principal.md` e `tabela_principal.tex`): Média $\pm$ desvio padrão, mínimo, mediana, gap para a instância pequena ($gap = (h - h^*)/h^*$) e tempo de CPU.
2. **Boxplots Comparativos** (`boxplot_pequena.png`, `boxplot_media.png`, `boxplot_grande.png`).
3. **Curvas de Convergência** (`convergencia_pequena.png`, `convergencia_media.png`, `convergencia_grande.png`): Mediana das execuções ao longo do número de avaliações.
4. **Taxa de Aceitação do Simulated Annealing** (`sa_aceitacao_*.png`).
5. **Evolução de Média e Diversidade do Genetic Algorithm** (`ga_diversidade_*.png`).
6. **Testes Estatísticos Não-Paramétricos** (`testes_estatisticos.md`): Testes bilaterais de Mann-Whitney / Wilcoxon com estatística $U$ e $p$-valores para todas as comparações par a par.
7. **Tabela T2 de Vizinhanças** (`tabela_t2_vizinhancas.md`).
