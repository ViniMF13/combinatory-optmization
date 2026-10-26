#!/usr/bin/env python3
import os
os.environ.setdefault('MPLCONFIGDIR', os.path.abspath('.cache/matplotlib'))
import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from scipy.stats import mannwhitneyu

sns.set_theme(style="whitegrid", palette="tab10")
plt.rcParams.update({'font.size': 12})

def resolve_input(fname):
    if os.path.exists(os.path.join("out", fname)):
        return os.path.join("out", fname)
    return fname

def out_path(fname):
    os.makedirs("out", exist_ok=True)
    return os.path.join("out", fname)

def analisar_tabela_principal(csv_path=None):
    if csv_path is None:
        csv_path = resolve_input("resultados_experimentos.csv")

    if not os.path.exists(csv_path):
        print(f"[-] Arquivo '{csv_path}' não encontrado.")
        return None

    df = pd.read_csv(csv_path)
    otimo_pequena = 1875.0

    print("\n" + "="*80)
    print("TABELA PRINCIPAL DE RESULTADOS (Seção 5 do PDF)")
    print("="*80)

    linhas = []
    for inst in ["pequena", "media", "grande"]:
        sub_df = df[df['Instancia'] == inst]
        if sub_df.empty:
            continue
        
        for alg in sub_df['Algoritmo'].unique():
            alg_df = sub_df[sub_df['Algoritmo'] == alg]
            mean_c = alg_df['MelhorCusto'].mean()
            std_c = alg_df['MelhorCusto'].std()
            min_c = alg_df['MelhorCusto'].min()
            median_c = alg_df['MelhorCusto'].median()
            mean_evals = int(alg_df['Avaliacoes'].mean()) if 'Avaliacoes' in alg_df else 0
            mean_steps = alg_df['PassosMelhoria'].mean() if 'PassosMelhoria' in alg_df else 0
            mean_restarts = alg_df['Restarts'].mean() if 'Restarts' in alg_df else 0

            gap = ((min_c - otimo_pequena) / otimo_pequena * 100.0) if inst == "pequena" else np.nan

            linhas.append({
                "Instancia": inst,
                "Algoritmo": alg,
                "Média ± DP": f"{mean_c:.1f} ± {std_c:.1f}",
                "Mediana": f"{median_c:.1f}",
                "Melhor (Min)": f"{min_c:.1f}",
                "Gap Ótimo (%)": f"{gap:.2f}%" if not np.isnan(gap) else "-",
                "Avaliações da FO": mean_evals,
                "Passos Melhoria": f"{mean_steps:.1f}",
                "Restarts": f"{mean_restarts:.1f}"
            })

    res_df = pd.DataFrame(linhas)
    print(res_df.to_string(index=False))

    # Salva em formato Markdown e LaTeX dentro de out/
    res_df.to_markdown(out_path("tabela_principal.md"), index=False)
    try:
        with open(out_path("tabela_principal.tex"), "w") as f:
            f.write(res_df.to_latex(index=False))
    except Exception:
        cols = " & ".join(res_df.columns) + " \\\\\n\\hline\n"
        rows = "\n".join(" & ".join(str(val) for val in row) + " \\\\" for row in res_df.values)
        latex_str = "\\begin{tabular}{" + "l" * len(res_df.columns) + "}\n\\hline\n" + cols + rows + "\n\\hline\n\\end{tabular}\n"
        with open(out_path("tabela_principal.tex"), "w") as f:
            f.write(latex_str)
    print(f"[+] Tabelas salvas em '{out_path('tabela_principal.md')}' e '{out_path('tabela_principal.tex')}'.")
    return df

def gerar_boxplots(df):
    if df is None: return
    for inst in df['Instancia'].unique():
        sub_df = df[df['Instancia'] == inst]
        plt.figure(figsize=(10, 6))
        sns.boxplot(x='Algoritmo', y='MelhorCusto', data=sub_df, showmeans=True,
                    meanprops={"marker":"o", "markerfacecolor":"red", "markeredgecolor":"red"})
        plt.title(f'Comparação de Desempenho - Instância {inst.capitalize()}', fontsize=14, fontweight='bold')
        plt.ylabel('Custo h (Atraso Ponderado Total)')
        plt.xlabel('Algoritmo')
        plt.xticks(rotation=25, ha='right')
        plt.tight_layout()
        out_name = out_path(f'boxplot_{inst}.png')
        plt.savefig(out_name, dpi=300)
        plt.close()
        print(f"[+] Boxplot gerado: {out_name}")

def realizar_testes_estatisticos(df):
    if df is None: return
    linhas_teste = []
    print("\n" + "="*80)
    print("TESTES ESTATÍSTICOS NÃO-PARAMÉTRICOS (Mann-Whitney U / Wilcoxon)")
    print("="*80)

    for inst in df['Instancia'].unique():
        sub_df = df[df['Instancia'] == inst]
        algs = list(sub_df['Algoritmo'].unique())

        for i in range(len(algs)):
            for j in range(i + 1, len(algs)):
                a1 = algs[i]
                a2 = algs[j]
                v1 = sub_df[sub_df['Algoritmo'] == a1]['MelhorCusto'].values
                v2 = sub_df[sub_df['Algoritmo'] == a2]['MelhorCusto'].values

                if len(v1) > 1 and len(v2) > 1:
                    stat, p_val = mannwhitneyu(v1, v2, alternative='two-sided')
                    sig = "Sim (p < 0.05)" if p_val < 0.05 else "Não (p >= 0.05)"
                    linhas_teste.append({
                        "Instancia": inst,
                        "Comparação": f"{a1} vs {a2}",
                        "Estatística U": stat,
                        "p-valor": f"{p_val:.4e}",
                        "Significativo?": sig
                    })

    test_df = pd.DataFrame(linhas_teste)
    print(test_df.to_string(index=False))
    test_df.to_markdown(out_path("testes_estatisticos.md"), index=False)
    print(f"[+] Testes estatísticos salvos em '{out_path('testes_estatisticos.md')}'.")

def gerar_curvas_convergencia(conv_csv=None):
    if conv_csv is None:
        conv_csv = resolve_input("curvas_convergencia.csv")

    if not os.path.exists(conv_csv):
        print(f"[-] Arquivo '{conv_csv}' não encontrado.")
        return

    df = pd.read_csv(conv_csv)
    for inst in df['Instancia'].unique():
        sub = df[df['Instancia'] == inst]
        plt.figure(figsize=(10, 6))

        for alg in sub['Algoritmo'].unique():
            alg_sub = sub[sub['Algoritmo'] == alg]
            # Agrupa por avaliações e calcula a mediana
            median_curve = alg_sub.groupby('Avaliacoes')['MelhorCusto'].median().reset_index()
            # Ordena e garante curva monotônica não crescente
            median_curve = median_curve.sort_values('Avaliacoes')
            median_curve['MelhorCusto'] = median_curve['MelhorCusto'].cummin()
            plt.plot(median_curve['Avaliacoes'], median_curve['MelhorCusto'], label=alg, linewidth=2)

        plt.title(f'Curvas de Convergência (Mediana) - Instância {inst.capitalize()}', fontsize=14, fontweight='bold')
        plt.xlabel('Número de Avaliações (Orçamento)')
        plt.ylabel('Melhor Custo h Encontrado')
        plt.legend(bbox_to_anchor=(1.04, 1), loc="upper left")
        plt.tight_layout()
        out_name = out_path(f'convergencia_{inst}.png')
        plt.savefig(out_name, dpi=300)
        plt.close()
        print(f"[+] Curva de convergência gerada: {out_name}")

def gerar_graficos_sa(sa_csv=None):
    if sa_csv is None:
        sa_csv = resolve_input("sa_aceitacao.csv")

    if not os.path.exists(sa_csv): return
    df = pd.read_csv(sa_csv)
    for inst in df['Instancia'].unique():
        sub = df[df['Instancia'] == inst]
        plt.figure(figsize=(10, 5))
        mean_acc = sub.groupby('Avaliacoes')['TaxaAceitacaoPiores'].mean().reset_index()
        plt.plot(mean_acc['Avaliacoes'], mean_acc['TaxaAceitacaoPiores'], color='darkorange', linewidth=2, label='Taxa de Aceitação (Piores)')
        plt.title(f'Simulated Annealing: Taxa de Aceitação de Movimentos Piores - {inst.capitalize()}', fontsize=14, fontweight='bold')
        plt.xlabel('Número de Avaliações')
        plt.ylabel('Taxa de Aceitação de Piora')
        plt.ylim(0, 1.05)
        plt.legend()
        plt.tight_layout()
        out_name = out_path(f'sa_aceitacao_{inst}.png')
        plt.savefig(out_name, dpi=300)
        plt.close()
        print(f"[+] Gráfico de aceitação SA gerado: {out_name}")

def gerar_graficos_ga(ga_csv=None):
    if ga_csv is None:
        ga_csv = resolve_input("ga_metricas.csv")

    if not os.path.exists(ga_csv): return
    df = pd.read_csv(ga_csv)
    for inst in df['Instancia'].unique():
        sub = df[df['Instancia'] == inst]
        fig, ax1 = plt.subplots(figsize=(10, 6))

        mean_sub = sub.groupby('Avaliacoes')[['MelhorCusto', 'MediaPopulacao', 'Diversidade']].mean().reset_index()

        ax1.plot(mean_sub['Avaliacoes'], mean_sub['MelhorCusto'], label='Melhor Indivíduo (h)', color='green', linewidth=2)
        ax1.plot(mean_sub['Avaliacoes'], mean_sub['MediaPopulacao'], label='Média da População (h)', color='blue', linestyle='--', linewidth=2)
        ax1.set_xlabel('Número de Avaliações')
        ax1.set_ylabel('Custo h (Atraso)', color='black')

        ax2 = ax1.twinx()
        ax2.plot(mean_sub['Avaliacoes'], mean_sub['Diversidade'], label='Diversidade da População', color='purple', linestyle=':', linewidth=2)
        ax2.set_ylabel('Diversidade (Cromossomos Únicos / N)', color='purple')
        ax2.set_ylim(0, 1.05)

        lines1, labels1 = ax1.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper right')

        plt.title(f'Genetic Algorithm: Média vs Melhor e Perda de Diversidade - {inst.capitalize()}', fontsize=14, fontweight='bold')
        plt.tight_layout()
        out_name = out_path(f'ga_diversidade_{inst}.png')
        plt.savefig(out_name, dpi=300)
        plt.close()
        print(f"[+] Gráfico de diversidade GA gerado: {out_name}")

def analisar_t2_vizinhancas(csv_path=None):
    if csv_path is None:
        csv_path = resolve_input("comparacao_vizinhancas.csv")

    if not os.path.exists(csv_path): return
    df = pd.read_csv(csv_path)
    print("\n" + "="*80)
    print("TAREFA T2: COMPARAÇÃO DAS VIZINHANÇAS (Swap vs Insertion vs 2-opt)")
    print("="*80)
    stats = df.groupby(['Instancia', 'Vizinhanca'])[['MelhorCusto', 'TempoMs', 'PassosMelhoria', 'Restarts']].mean().reset_index()
    print(stats.to_string(index=False))
    stats.to_markdown(out_path("tabela_t2_vizinhancas.md"), index=False)
    print(f"[+] Tabela T2 salva em '{out_path('tabela_t2_vizinhancas.md')}'.")

if __name__ == "__main__":
    print(">>> Iniciando Análise de Resultados e Geração de Figuras para o Relatório <<<")
    df = analisar_tabela_principal()
    if df is not None:
        gerar_boxplots(df)
        realizar_testes_estatisticos(df)
        gerar_curvas_convergencia()
        gerar_graficos_sa()
        gerar_graficos_ga()
    analisar_t2_vizinhancas()
    print("\n[OK] Todas as análises, tabelas e figuras foram geradas e salvas na pasta 'out/'!")
