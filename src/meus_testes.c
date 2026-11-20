// meus_testes.c - testes extra feitos por nos, alem dos do professor.
// Cobrem os casos limite que o unit_tests.c nao testa (poligono a crescer,
// remocoes, estabilidade das ordenacoes, contadores exatos, etc.).
//
// Compilar e correr: make meus-testes

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "primitive_internal.h"
#include "vector_scene.h"

static int total = 0;
static int falhas = 0;

#define VERIFICA(condicao, mensagem)                                          \
  do {                                                                        \
    total++;                                                                  \
    if (!(condicao)) {                                                        \
      falhas++;                                                               \
      printf("  [FALHOU] %s (linha %d): %s\n", __func__, __LINE__, mensagem); \
    }                                                                         \
  } while (0)

static const Color PRETO = {0, 0, 0};

// Cria uma cena com n circulos. modo_z: 0 = z repetidos, 1 = ordenados,
// 2 = ordem inversa
static VectorScene* cena_com_circulos(int n, int modo_z) {
  VectorScene* cena = vector_scene_create(100, 100, PRETO);
  for (int i = 0; i < n; i++) {
    int z = i % 5;               // modo 0: muitos z repetidos
    if (modo_z == 1) z = i;      // ja ordenado
    if (modo_z == 2) z = n - i;  // ordem inversa
    vector_scene_add_primitive(
        cena, primitive_create_circle(-1, (Point){50, 50}, 5, PRETO, 1.0, z));
  }
  return cena;
}

// ---------------------------------------------------------------------------

static void teste_poligono_cresce_e_remove(void) {
  printf("- poligono: crescimento e remocao de vertices\n");
  Primitive* poly = primitive_create_polygon(1, PRETO, 1.0, 0);

  // 20 vertices obrigam a capacidade a crescer varias vezes (4, 8, 16, 32)
  for (int i = 0; i < 20; i++) {
    VERIFICA(polygon_add_vertex(poly, (Point){i, 2 * i}), "adicionar vertice");
  }
  VERIFICA(polygon_get_num_vertices(poly) == 20, "20 vertices");
  VERIFICA(poly->geom.polygon.vertices[19].y == 38, "ultimo vertice correto");

  VERIFICA(polygon_remove_vertex(poly, 0), "remover o primeiro");
  VERIFICA(poly->geom.polygon.vertices[0].x == 1,
           "o segundo passou a primeiro");
  VERIFICA(polygon_remove_vertex(poly, 9), "remover um do meio");
  VERIFICA(polygon_remove_vertex(poly, 17), "remover o ultimo (indice 17)");
  VERIFICA(polygon_get_num_vertices(poly) == 17, "restam 17");
  VERIFICA(!polygon_remove_vertex(poly, 17), "indice fora do limite falha");

  while (polygon_get_num_vertices(poly) > 0) {
    polygon_remove_vertex(poly, 0);
  }
  VERIFICA(!polygon_remove_vertex(poly, 0), "remover de poligono vazio falha");
  VERIFICA(polygon_add_vertex(poly, (Point){1, 1}), "voltar a adicionar");
  primitive_destroy(poly);

  Primitive* circulo =
      primitive_create_circle(2, (Point){0, 0}, 1, PRETO, 1, 0);
  VERIFICA(!polygon_add_vertex(circulo, (Point){1, 1}), "nao e poligono");
  VERIFICA(!polygon_remove_vertex(circulo, 0), "nao e poligono (remover)");
  primitive_destroy(circulo);
}

static void teste_aabb_e_transformacoes(void) {
  printf("- AABB, translate e scale\n");

  Primitive* ret =
      primitive_create_rectangle(1, (Point){10, 20}, 30, 40, PRETO, 1.0, 0);
  BoundingBox caixa = primitive_get_aabb(ret);
  VERIFICA(caixa.min.x == 10 && caixa.min.y == 20, "AABB retangulo (min)");
  VERIFICA(caixa.max.x == 40 && caixa.max.y == 60, "AABB retangulo (max)");
  primitive_scale(ret, 2.0);
  caixa = primitive_get_aabb(ret);
  VERIFICA(caixa.max.x == 70 && caixa.max.y == 100, "retangulo escalado");
  primitive_translate(ret, 5, -5);
  VERIFICA(ret->is_aabb_dirty, "translate marca a AABB como suja");
  caixa = primitive_get_aabb(ret);
  VERIFICA(caixa.min.x == 15 && caixa.min.y == 15, "retangulo transladado");
  primitive_destroy(ret);

  Primitive* seg = primitive_create_segment(2, (Point){50, 10}, (Point){20, 40},
                                            PRETO, 1, 0);
  caixa = primitive_get_aabb(seg);
  VERIFICA(caixa.min.x == 18 && caixa.min.y == 8, "AABB segmento (min)");
  VERIFICA(caixa.max.x == 52 && caixa.max.y == 42, "AABB segmento (max)");
  primitive_destroy(seg);

  seg = primitive_create_segment(3, (Point){0, 0}, (Point){10, 0}, PRETO, 1, 0);
  primitive_scale(seg, 2.0);
  VERIFICA(seg->geom.segment.p1.x == -5 && seg->geom.segment.p2.x == 15,
           "segmento escalado em torno do ponto medio");
  primitive_destroy(seg);

  Primitive* tri = primitive_create_polygon(4, PRETO, 1.0, 0);
  polygon_add_vertex(tri, (Point){0, 0});
  polygon_add_vertex(tri, (Point){10, 0});
  polygon_add_vertex(tri, (Point){5, 10});
  caixa = primitive_get_aabb(tri);
  VERIFICA(caixa.max.x == 10 && caixa.max.y == 10, "AABB poligono");
  polygon_add_vertex(tri, (Point){20, -5});
  VERIFICA(tri->is_aabb_dirty, "adicionar vertice suja a AABB");
  caixa = primitive_get_aabb(tri);
  VERIFICA(caixa.max.x == 20 && caixa.min.y == -5, "AABB recalculada");
  polygon_remove_vertex(tri, 3);
  primitive_scale(tri, 2.0);  // centro (5, 5): o vertice (0,0) vai para (-5,-5)
  VERIFICA(tri->geom.polygon.vertices[0].x == -5 &&
               tri->geom.polygon.vertices[0].y == -5,
           "poligono escalado em torno do centro da AABB");
  primitive_destroy(tri);

  // Poligono vazio: a AABB nao deve rebentar
  Primitive* vazio = primitive_create_polygon(5, PRETO, 1.0, 0);
  caixa = primitive_get_aabb(vazio);
  VERIFICA(caixa.max.x == 0 && caixa.max.y == 0, "AABB de poligono vazio");
  primitive_destroy(vazio);
}

static void teste_pertenca_de_pontos(void) {
  printf("- primitive_contains_point\n");

  Primitive* ret =
      primitive_create_rectangle(1, (Point){0, 0}, 10, 10, PRETO, 1.0, 0);
  VERIFICA(primitive_contains_point(ret, (Point){5, 5}), "dentro do retangulo");
  VERIFICA(primitive_contains_point(ret, (Point){10, 10}), "canto conta");
  VERIFICA(!primitive_contains_point(ret, (Point){10.1, 5}), "fora");
  primitive_destroy(ret);

  Primitive* seg =
      primitive_create_segment(2, (Point){0, 0}, (Point){10, 10}, PRETO, 1, 0);
  VERIFICA(primitive_contains_point(seg, (Point){5, 5}), "ponto no segmento");
  VERIFICA(primitive_contains_point(seg, (Point){5, 6}), "ponto perto");
  VERIFICA(!primitive_contains_point(seg, (Point){5, 9}), "ponto longe");
  primitive_destroy(seg);

  Primitive* tri = primitive_create_polygon(3, PRETO, 1.0, 0);
  VERIFICA(!primitive_contains_point(tri, (Point){1, 1}), "poligono vazio");
  polygon_add_vertex(tri, (Point){0, 0});
  polygon_add_vertex(tri, (Point){10, 0});
  VERIFICA(!primitive_contains_point(tri, (Point){5, 0}), "so 2 vertices");
  polygon_add_vertex(tri, (Point){5, 10});
  VERIFICA(primitive_contains_point(tri, (Point){5, 3}), "dentro do triangulo");
  VERIFICA(!primitive_contains_point(tri, (Point){20, 20}),
           "fora do triangulo");
  primitive_destroy(tri);

  VERIFICA(!primitive_contains_point(NULL, (Point){0, 0}), "NULL");
}

static void teste_cena_ids_e_remocao(void) {
  printf("- cena: ids, crescimento e remocao\n");
  VectorScene* cena = vector_scene_create(100, 100, PRETO);

  // 20 formas passam a capacidade inicial (8) e obrigam a realloc
  for (int i = 0; i < 20; i++) {
    Primitive* p = primitive_create_circle(-1, (Point){1, 1}, 1, PRETO, 1, 0);
    int id = vector_scene_add_primitive(cena, p);
    VERIFICA(id == i + 1, "ids automaticos 1, 2, 3, ...");
  }
  VERIFICA(vector_scene_get_count(cena) == 20, "20 formas");

  // Um id explicito alto nao pode ser repetido pelos ids automaticos
  Primitive* explicita =
      primitive_create_circle(100, (Point){1, 1}, 1, PRETO, 1, 0);
  VERIFICA(vector_scene_add_primitive(cena, explicita) == 100, "id explicito");
  Primitive* automatica =
      primitive_create_circle(-1, (Point){1, 1}, 1, PRETO, 1, 0);
  VERIFICA(vector_scene_add_primitive(cena, automatica) == 101,
           "o id automatico seguinte e 101");

  VERIFICA(vector_scene_remove_primitive(cena, 1), "remover a primeira");
  VERIFICA(vector_scene_remove_primitive(cena, 10), "remover uma do meio");
  VERIFICA(vector_scene_remove_primitive(cena, 101), "remover a ultima");
  VERIFICA(!vector_scene_remove_primitive(cena, 999), "id inexistente");
  VERIFICA(vector_scene_get_count(cena) == 19, "restam 19");
  VERIFICA(vector_scene_get_primitive_by_id(cena, 10) == NULL,
           "10 ja nao existe");
  VERIFICA(vector_scene_get_primitive_by_id(cena, 2) != NULL, "2 continua");

  VERIFICA(vector_scene_set_z_index(cena, 2, 7), "mudar z");
  VERIFICA(
      primitive_get_z_index(vector_scene_get_primitive_by_id(cena, 2)) == 7,
      "z mudou");
  VERIFICA(!vector_scene_set_z_index(cena, 999, 7),
           "mudar z de id inexistente");
  VERIFICA(!vector_scene_translate_primitive(cena, 999, 1, 1),
           "transladar id inexistente");

  vector_scene_destroy(cena);
}

static void teste_ordenacao(void) {
  printf("- ordenacao: ordem, estabilidade e contadores\n");

  // z muito repetidos (0..4): a ordenacao tem de ser estavel, ou seja, entre
  // formas com o mesmo z mantem-se a ordem de insercao (ids crescentes)
  for (int usar_merge = 0; usar_merge <= 1; usar_merge++) {
    VectorScene* cena = cena_com_circulos(200, 0);
    SortStats st;
    vector_scene_sort_by_z(cena, usar_merge, &st);

    bool esta_ordenada = true;
    for (size_t i = 1; i < vector_scene_get_count(cena); i++) {
      Primitive* antes = vector_scene_get_primitive_by_index(cena, i - 1);
      Primitive* depois = vector_scene_get_primitive_by_index(cena, i);
      if (antes->z_index > depois->z_index) esta_ordenada = false;
      if (antes->z_index == depois->z_index && antes->id > depois->id) {
        esta_ordenada = false;
      }
    }
    VERIFICA(esta_ordenada, "ordenada por z e estavel");
    vector_scene_destroy(cena);
  }

  // Contadores exatos do insertion sort (n = 100)
  SortStats st;
  VectorScene* cena = cena_com_circulos(100, 1);  // ja ordenado
  vector_scene_sort_by_z(cena, false, &st);
  VERIFICA(st.comparisons == 99 && st.swaps == 0, "insertion: melhor caso");
  vector_scene_destroy(cena);

  cena = cena_com_circulos(100, 2);  // ordem inversa
  vector_scene_sort_by_z(cena, false, &st);
  VERIFICA(st.comparisons == 4950 && st.swaps == 4950,
           "insertion: pior caso = n(n-1)/2");
  vector_scene_destroy(cena);

  // Sem estatisticas (NULL) nao pode rebentar, nem num algoritmo nem noutro
  cena = cena_com_circulos(50, 2);
  vector_scene_sort_by_z(cena, false, NULL);
  vector_scene_destroy(cena);
  cena = cena_com_circulos(50, 2);
  vector_scene_sort_by_z(cena, true, NULL);
  vector_scene_destroy(cena);

  // Cenas com 0 ou 1 formas: os contadores ficam a zero
  cena = cena_com_circulos(0, 0);
  st.comparisons = 7;
  vector_scene_sort_by_z(cena, true, &st);
  VERIFICA(st.comparisons == 0 && st.swaps == 0, "cena vazia");
  vector_scene_destroy(cena);
  cena = cena_com_circulos(1, 0);
  vector_scene_sort_by_z(cena, false, &st);
  VERIFICA(st.comparisons == 0 && st.swaps == 0, "cena com 1 forma");
  vector_scene_destroy(cena);
}

static void teste_picking(void) {
  printf("- picking direto e em duas fases\n");
  VectorScene* cena = vector_scene_create(200, 200, PRETO);
  vector_scene_add_primitive(
      cena, primitive_create_circle(1, (Point){50, 50}, 30, PRETO, 1, 1));
  vector_scene_add_primitive(
      cena, primitive_create_circle(2, (Point){50, 50}, 10, PRETO, 1, 5));
  vector_scene_add_primitive(
      cena,
      primitive_create_rectangle(3, (Point){120, 120}, 40, 40, PRETO, 1, 3));

  PickStats direta, otimizada;
  Point no_centro = {50, 50};
  Primitive* a =
      vector_scene_pick_top_primitive(cena, no_centro, false, &direta);
  Primitive* b =
      vector_scene_pick_top_primitive(cena, no_centro, true, &otimizada);
  VERIFICA(a != NULL && a == b, "as duas estrategias escolhem a mesma forma");
  VERIFICA(primitive_get_id(a) == 2, "ganha a de maior z_index");
  VERIFICA(direta.aabb_tests == 0, "a direta nao usa AABB");

  // Ponto longe de tudo: a otimizada nunca chega ao teste exato
  Point vazio = {190, 10};
  VERIFICA(
      vector_scene_pick_top_primitive(cena, vazio, true, &otimizada) == NULL,
      "ponto fora de tudo");
  VERIFICA(otimizada.aabb_tests == 3 && otimizada.exact_tests == 0,
           "3 testes de AABB e nenhum exato");
  VERIFICA(vector_scene_pick_top_primitive(cena, vazio, false, &direta) == NULL,
           "ponto fora de tudo (direta)");
  VERIFICA(direta.exact_tests == 3, "a direta faz 3 testes exatos");

  VERIFICA(vector_scene_pick_top_primitive(NULL, vazio, true, &direta) == NULL,
           "cena NULL");
  vector_scene_destroy(cena);
}

static void teste_render_e_ficheiros(void) {
  printf("- render e gravar/carregar ficheiros\n");

  VectorScene* vazia = vector_scene_create(20, 10, (Color){9, 8, 7});
  VERIFICA(vector_scene_render(vazia, "meu_teste_vazio.ppm"), "render vazio");
  vector_scene_destroy(vazia);
  remove("meu_teste_vazio.ppm");

  // Cena com um poligono: gravar e carregar tem de dar o mesmo poligono
  VectorScene* cena = vector_scene_create(100, 100, PRETO);
  Primitive* tri = primitive_create_polygon(-1, (Color){200, 0, 0}, 0.5, 3);
  polygon_add_vertex(tri, (Point){10, 10});
  polygon_add_vertex(tri, (Point){60, 10});
  polygon_add_vertex(tri, (Point){30, 50});
  int id = vector_scene_add_primitive(cena, tri);
  VERIFICA(vector_scene_save_text(cena, "meu_teste.vsc"), "gravar");
  vector_scene_destroy(cena);

  VectorScene* lida = vector_scene_load_text("meu_teste.vsc");
  VERIFICA(lida != NULL, "carregar");
  if (lida != NULL) {
    Primitive* p = vector_scene_get_primitive_by_id(lida, id);
    VERIFICA(p != NULL && polygon_get_num_vertices(p) == 3, "3 vertices");
    if (p != NULL && polygon_get_num_vertices(p) == 3) {
      VERIFICA(p->geom.polygon.vertices[0].x == 10 &&
                   p->geom.polygon.vertices[0].y == 10,
               "primeiro vertice igual ao original");
      VERIFICA(p->geom.polygon.vertices[2].x == 30 &&
                   p->geom.polygon.vertices[2].y == 50,
               "ultimo vertice igual ao original");
    }
    vector_scene_destroy(lida);
  }
  remove("meu_teste.vsc");
}


static void teste_regressoes(void) {
  printf("- fronteiras, tolerancia dos segmentos e IDs extremos\n");
  Primitive* poly = primitive_create_polygon(1, PRETO, 1, 0);
  Point vertices[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  for (size_t i = 0; i < 4; i++) polygon_add_vertex(poly, vertices[i]);
  for (size_t i = 0; i < 4; i++) {
    Point a = vertices[i], b = vertices[(i + 1) % 4];
    VERIFICA(primitive_contains_point(poly, a), "vertice incluido");
    VERIFICA(primitive_contains_point(poly,
             (Point){(a.x + b.x) / 2, (a.y + b.y) / 2}), "aresta incluida");
  }
  VERIFICA(!primitive_contains_point(poly, (Point){10.01, 5}), "fora da aresta");
  primitive_destroy(poly);

  Point extremos[] = {{10, 0}, {0, 10}, {10, 10}, {0, 0}};
  for (size_t k = 0; k < 4; k++) {
    VectorScene* cena = vector_scene_create(30, 30, PRETO);
    Primitive* seg = primitive_create_segment(1, (Point){0, 0}, extremos[k], PRETO, 1, 0);
    vector_scene_add_primitive(cena, seg);
    for (int x = -6; x <= 26; x++) {
      for (int y = -6; y <= 26; y++) {
        Point pt = {x / 2.0, y / 2.0};
        VERIFICA(vector_scene_pick_top_primitive(cena, pt, false, NULL) ==
                 vector_scene_pick_top_primitive(cena, pt, true, NULL),
                 "picking direto e otimizado concordam junto ao segmento");
      }
    }
    vector_scene_destroy(cena);
  }

  VectorScene* cena = vector_scene_create(20, 20, PRETO);
  Primitive* a = primitive_create_circle(7, (Point){0, 0}, 1, PRETO, 1, 0);
  Primitive* b = primitive_create_circle(7, (Point){0, 0}, 1, PRETO, 1, 0);
  VERIFICA(vector_scene_add_primitive(cena, a) == 7, "primeiro ID aceite");
  VERIFICA(vector_scene_add_primitive(cena, b) == -1, "ID repetido rejeitado");
  primitive_destroy(b);
  VERIFICA(vector_scene_add_primitive(cena, a) == -1, "ponteiro repetido rejeitado");
  Primitive* maximo = primitive_create_circle(INT_MAX, (Point){0, 0}, 1, PRETO, 1, 0);
  VERIFICA(vector_scene_add_primitive(cena, maximo) == INT_MAX, "INT_MAX aceite sem overflow");
  Primitive* automatica = primitive_create_circle(-1, (Point){0, 0}, 1, PRETO, 1, 0);
  int id = vector_scene_add_primitive(cena, automatica);
  VERIFICA(id >= 0 && id != 7 && id != INT_MAX, "ID livre depois de INT_MAX");
  b = primitive_create_circle(INT_MAX, (Point){0, 0}, 1, PRETO, 1, 0);
  VERIFICA(vector_scene_add_primitive(cena, b) == -1, "INT_MAX repetido rejeitado");
  primitive_destroy(b);
  VERIFICA(vector_scene_remove_primitive(cena, 7), "remove ID antigo");
  b = primitive_create_circle(7, (Point){0, 0}, 1, PRETO, 1, 0);
  VERIFICA(vector_scene_add_primitive(cena, b) == 7, "ID removido pode ser reutilizado");
  vector_scene_destroy(cena);

  FILE* f = fopen("ids_repetidos_teste.vsc", "w");
  VERIFICA(f != NULL, "ficheiro temporario");
  if (f) {
    fputs("SCENE 20 20 0 0 0\nCIRC 7 0 0 0 0 1 0 0 1\nCIRC 7 0 0 0 0 1 1 1 1\n", f);
    fclose(f);
    cena = vector_scene_load_text("ids_repetidos_teste.vsc");
    VERIFICA(cena == NULL, "loader rejeita IDs repetidos e liberta a cena");
    vector_scene_destroy(cena);
    remove("ids_repetidos_teste.vsc");
  }
}

int main(void) {
  printf("=== Testes proprios ===\n");
  teste_poligono_cresce_e_remove();
  teste_aabb_e_transformacoes();
  teste_pertenca_de_pontos();
  teste_cena_ids_e_remocao();
  teste_ordenacao();
  teste_picking();
  teste_render_e_ficheiros();
  teste_regressoes();

  printf("=== %d/%d passaram (%d falhas) ===\n", total - falhas, total, falhas);
  return falhas == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
