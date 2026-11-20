// benchmark.c - mede o tempo das ordenações e do picking
//
// Como usar:
//   benchmark sort  -> testa o insertion sort e o merge sort
//   benchmark pick  -> testa o picking direto e o de duas fases
// Os resultados saem em formato CSV (separado por ;) para abrir no Excel.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vector_scene.h"

#define LARGURA 1000
#define ALTURA 1000

// devolve um número aleatório entre min e max
double aleatorio(double min, double max) {
  return min + (max - min) * (rand() / (double)RAND_MAX);
}

// cria uma cena com n formas pequenas (metade círculos, metade retângulos)
// tipo_z: 0 = z_index aleatório, 1 = já ordenado, 2 = ordem inversa
VectorScene* criar_cena(int n, int tipo_z) {
  srand(42);  // semente fixa: assim os dois algoritmos ordenam os mesmos dados
  VectorScene* cena = vector_scene_create(LARGURA, ALTURA, (Color){0, 0, 0});

  for (int i = 0; i < n; i++) {
    int z;
    if (tipo_z == 1) {
      z = i;
    } else if (tipo_z == 2) {
      z = n - i;
    } else {
      z = rand() % n;
    }

    Point pos = {aleatorio(0, LARGURA), aleatorio(0, ALTURA)};
    double tamanho = aleatorio(5, 30);
    Color vermelho = {255, 0, 0};

    Primitive* forma;
    if (i % 2 == 0) {
      forma = primitive_create_circle(-1, pos, tamanho, vermelho, 1.0, z);
    } else {
      forma = primitive_create_rectangle(-1, pos, tamanho, tamanho, vermelho,
                                         1.0, z);
    }
    vector_scene_add_primitive(cena, forma);
  }
  return cena;
}

void testar_ordenacao(void) {
  int tamanhos[] = {100, 300, 1000, 3000, 10000, 30000, 100000};
  const char* nome_tipo[] = {"aleatorio", "ordenado", "invertido"};

  printf("N;cenario;algoritmo;tempo_s;comparacoes;trocas\n");

  for (size_t t = 0; t < sizeof(tamanhos) / sizeof(tamanhos[0]); t++) {
    for (int tipo = 0; tipo < 3; tipo++) {
      for (int merge = 0; merge <= 1; merge++) {
        // com N pequeno, uma ordenação demora menos do que o relógio
        // consegue medir, por isso repetimos várias vezes e fazemos a média
        int repeticoes = 1;
        if (tamanhos[t] <= 1000) {
          repeticoes = 1000;
        } else if (tamanhos[t] <= 10000) {
          repeticoes = 20;
        }

        double tempo_total = 0;
        SortStats st;

        for (int r = 0; r < repeticoes; r++) {
          // cena nova de cada vez, senão a ordenação seguinte
          // recebia os dados já ordenados
          VectorScene* cena = criar_cena(tamanhos[t], tipo);

          clock_t inicio = clock();
          vector_scene_sort_by_z(cena, merge, &st);
          tempo_total += (double)(clock() - inicio) / CLOCKS_PER_SEC;

          vector_scene_destroy(cena);
        }

        // os contadores são iguais em todas as repetições (mesma semente),
        // só o tempo é que é uma média
        printf("%d;%s;%s;%.7f;%lu;%lu\n", tamanhos[t], nome_tipo[tipo],
               merge ? "merge" : "insertion", tempo_total / repeticoes,
               st.comparisons, st.swaps);
        fflush(stdout);  // escreve logo a linha (o N=100000 demora bastante)
      }
    }
  }
}

void testar_picking(void) {
  int tamanhos[] = {100, 300, 1000, 3000, 10000, 30000, 100000};
  int cliques = 10000;  // quantos pontos aleatórios testamos em cada cena

  printf("N;estrategia;tempo_s;testes_aabb;testes_exatos;acertos\n");

  for (size_t t = 0; t < sizeof(tamanhos) / sizeof(tamanhos[0]); t++) {
    VectorScene* cena = criar_cena(tamanhos[t], 0);
    vector_scene_sort_by_z(cena, true,
                           NULL);  // ordenamos uma vez antes dos cliques

    for (int duas_fases = 0; duas_fases <= 1; duas_fases++) {
      srand(1234);  // para as duas estratégias usarem os mesmos pontos
      unsigned long aabb = 0, exatos = 0, acertos = 0;

      clock_t inicio = clock();
      for (int c = 0; c < cliques; c++) {
        Point pt = {aleatorio(0, LARGURA), aleatorio(0, ALTURA)};
        PickStats st;
        if (vector_scene_pick_top_primitive(cena, pt, duas_fases, &st) !=
            NULL) {
          acertos++;
        }
        aabb += st.aabb_tests;
        exatos += st.exact_tests;
      }
      double tempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;

      printf("%d;%s;%f;%lu;%lu;%lu\n", tamanhos[t],
             duas_fases ? "duas_fases" : "direta", tempo, aabb, exatos,
             acertos);
      fflush(stdout);
    }
    vector_scene_destroy(cena);
  }
}

int main(int argc, char** argv) {
  if (argc >= 2 && strcmp(argv[1], "sort") == 0) {
    testar_ordenacao();
  } else if (argc >= 2 && strcmp(argv[1], "pick") == 0) {
    testar_picking();
  } else {
    printf("uso: %s sort | pick\n", argv[0]);
    return 1;
  }
  return 0;
}