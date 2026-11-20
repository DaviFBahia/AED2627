#include <stdio.h>
#include <stdlib.h>

#include "vector_scene.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Uso: %s entrada.vsc saida.ppm\n", argv[0]);
    return EXIT_FAILURE;
  }

  VectorScene* cena = vector_scene_load_text(argv[1]);
  if (cena == NULL) {
    fprintf(stderr, "Erro ao carregar a cena: %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  bool sucesso = vector_scene_render(cena, argv[2]);
  vector_scene_destroy(cena);

  if (!sucesso) {
    fprintf(stderr, "Erro ao gerar a imagem: %s\n", argv[2]);
    return EXIT_FAILURE;
  }

  printf("Imagem criada: %s\n", argv[2]);
  return EXIT_SUCCESS;
}
