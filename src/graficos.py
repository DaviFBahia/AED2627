"""Gráficos dos benchmarks de AED — Grupo P1B.
Executar a partir da pasta que contém 'resultados': python3 graficos.py
"""
import csv
from pathlib import Path

import matplotlib.pyplot as plt

PASTA = Path('resultados')


def ler_csv(nome):
    with (PASTA / nome).open(encoding='utf-8', newline='') as ficheiro:
        return list(csv.DictReader(ficheiro, delimiter=';'))


ordenacao = ler_csv('ordenacao.csv')
picking = ler_csv('picking.csv')
tamanhos = sorted({int(linha['N']) for linha in ordenacao})


def serie_sort(cenario, algoritmo, coluna):
    return [float(next(l[coluna] for l in ordenacao
                       if int(l['N']) == n and l['cenario'] == cenario
                       and l['algoritmo'] == algoritmo)) for n in tamanhos]


def serie_pick(estrategia, coluna):
    return [float(next(l[coluna] for l in picking
                       if int(l['N']) == n and l['estrategia'] == estrategia))
            for n in tamanhos]


# 1 — Três cenários da ordenação, com sete pontos reais em cada curva.
fig, eixos = plt.subplots(1, 3, figsize=(11.8, 3.55), sharex=True, sharey=True)
for eixo, cenario in zip(eixos, ['aleatorio', 'ordenado', 'invertido']):
    for algoritmo, nome, marca in [('insertion', 'Insertion Sort', 'o'),
                                   ('merge', 'Merge Sort', 's')]:
        eixo.plot(tamanhos, serie_sort(cenario, algoritmo, 'tempo_s'),
                  marker=marca, markersize=4, linewidth=1.5, label=nome)
    eixo.set_title(cenario.capitalize(), fontsize=10)
    eixo.set_xscale('log')
    eixo.set_yscale('log')
    eixo.grid(True, which='major', alpha=.3)
    eixo.set_xlabel('Número de primitivas (N)', fontsize=9)
    eixo.tick_params(labelsize=8)
eixos[0].set_ylabel('Tempo de execução (s)', fontsize=9)
fig.legend(*eixos[0].get_legend_handles_labels(), loc='upper center',
           ncol=2, bbox_to_anchor=(.5, 1.055), frameon=False, fontsize=9)
fig.tight_layout(rect=(0, 0, 1, .96))
fig.savefig(PASTA / 'ordenacao_tres_cenarios.png', dpi=200, bbox_inches='tight')
plt.close(fig)

# 2 — Comparações experimentais e fórmula do pior caso do Insertion Sort.
teorico = [n * (n - 1) // 2 for n in tamanhos]
medido = [int(x) for x in serie_sort('invertido', 'insertion', 'comparacoes')]
fig, ax = plt.subplots(figsize=(7.6, 3.7))
ax.plot(tamanhos, teorico, '--', linewidth=1.5, label='Teoria: N(N − 1)/2')
ax.scatter(tamanhos, medido, marker='o', s=25, zorder=5, label='Medições')
ax.set_xscale('log')
ax.set_yscale('log')
ax.set_xlabel('Número de primitivas (N)')
ax.set_ylabel('Comparações')
ax.set_title('Insertion Sort — cenário invertido')
ax.grid(True, which='major', alpha=.3)
ax.legend(fontsize=9)
fig.tight_layout()
fig.savefig(PASTA / 'ordenacao_teoria_vs_medicoes.png', dpi=200)
plt.close(fig)

# 3 — Número de testes geométricos exatos no picking.
fig, ax = plt.subplots(figsize=(7.6, 3.7))
for estrategia, label, marca in [('direta', 'Direto', 'o'),
                                 ('duas_fases', 'Duas fases (AABB)', 's')]:
    ax.plot(tamanhos, serie_pick(estrategia, 'testes_exatos'),
            marker=marca, markersize=5, linewidth=1.6, label=label)
ax.set_xscale('log')
ax.set_yscale('log')
ax.set_xlabel('Número de primitivas (N)')
ax.set_ylabel('Testes geométricos exatos')
ax.set_title('Picking — testes exatos realizados')
ax.grid(True, which='major', alpha=.3)
ax.legend(fontsize=9)
fig.tight_layout()
fig.savefig(PASTA / 'picking_testes_exatos.png', dpi=200)
plt.close(fig)

# 4 — Tempos de execução do picking.
fig, ax = plt.subplots(figsize=(7.6, 3.7))
for estrategia, label, marca in [('direta', 'Direto', 'o'),
                                 ('duas_fases', 'Duas fases (AABB)', 's')]:
    ax.plot(tamanhos, serie_pick(estrategia, 'tempo_s'),
            marker=marca, markersize=5, linewidth=1.6, label=label)
ax.set_xscale('log')
ax.set_yscale('log')
ax.set_xlabel('Número de primitivas (N)')
ax.set_ylabel('Tempo de execução (s)')
ax.set_title('Picking — comparação dos tempos')
ax.grid(True, which='major', alpha=.3)
ax.legend(fontsize=9)
fig.tight_layout()
fig.savefig(PASTA / 'picking_tempos.png', dpi=200)
plt.close(fig)

# 5 — Rácios derivados dos resultados, sem novas experiências.
with (PASTA / 'racios.csv').open('w', encoding='utf-8', newline='') as ficheiro:
    writer = csv.writer(ficheiro, delimiter=';')
    writer.writerow(['N', 'vantagem_merge_aleatorio_tempo_x',
                     'vantagem_merge_aleatorio_comparacoes_x',
                     'vantagem_duas_fases_tempo_x', 'reducao_testes_exatos_pct'])
    for i, n in enumerate(tamanhos):
        t_ins = serie_sort('aleatorio', 'insertion', 'tempo_s')[i]
        t_merge = serie_sort('aleatorio', 'merge', 'tempo_s')[i]
        c_ins = serie_sort('aleatorio', 'insertion', 'comparacoes')[i]
        c_merge = serie_sort('aleatorio', 'merge', 'comparacoes')[i]
        t_dir = serie_pick('direta', 'tempo_s')[i]
        t_duas = serie_pick('duas_fases', 'tempo_s')[i]
        ex_dir = serie_pick('direta', 'testes_exatos')[i]
        ex_duas = serie_pick('duas_fases', 'testes_exatos')[i]
        writer.writerow([n, f'{t_ins/t_merge:.3f}', f'{c_ins/c_merge:.3f}',
                         f'{t_dir/t_duas:.3f}', f'{100*(1-ex_duas/ex_dir):.3f}'])

print('Gerados quatro gráficos e a tabela de rácios em resultados/.')
print('Conferência da teoria:', 'OK' if medido == teorico else 'DIFERENÇAS')
