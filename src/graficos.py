
import csv
import matplotlib.pyplot as plt

# Ler os resultados da ordenacao
with open("resultados/ordenacao.csv") as ficheiro:
    dados = list(csv.DictReader(ficheiro, delimiter=";"))

# Criar um grafico para cada cenario
for cenario in ["aleatorio", "ordenado", "invertido"]:
    plt.figure(figsize=(7, 4))

    for algoritmo in ["insertion", "merge"]:
        linhas = [
            linha for linha in dados
            if linha["cenario"] == cenario
            and linha["algoritmo"] == algoritmo
        ]

        n = [int(linha["N"]) for linha in linhas]
        tempos = [float(linha["tempo_s"]) for linha in linhas]

        plt.plot(n, tempos, marker="o", label=algoritmo)

    plt.xscale("log")
    plt.yscale("symlog", linthresh=0.00001)
    plt.xlabel("Numero de primitivas")
    plt.ylabel("Tempo (segundos)")
    plt.title("Ordenacao - " + cenario)
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    plt.savefig("resultados/ordenacao_" + cenario + ".png")
    plt.close()

# Ler os resultados do picking
with open("resultados/picking.csv") as ficheiro:
    dados = list(csv.DictReader(ficheiro, delimiter=";"))

plt.figure(figsize=(7, 4))

for estrategia in ["direta", "duas_fases"]:
    linhas = [
        linha for linha in dados
        if linha["estrategia"] == estrategia
    ]

    n = [int(linha["N"]) for linha in linhas]
    tempos = [float(linha["tempo_s"]) for linha in linhas]

    plt.plot(n, tempos, marker="o", label=estrategia)

plt.xscale("log")
plt.xlabel("Numero de primitivas")
plt.ylabel("Tempo (segundos)")
plt.title("Comparacao do picking")
plt.legend()
plt.grid(True)
plt.tight_layout()

plt.savefig("resultados/picking.png")
plt.close()

print("Graficos criados!")
