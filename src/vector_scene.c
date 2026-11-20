/**
 * @file vector_scene.c
 * @brief Implementation skeleton of the VectorScene ADT (2D Vector Scene)
 *
 * Algoritmos e Estruturas de Dados 2026/2027
 * Departamento de Electrónica, Telecomunicações e Informática - UA
 */

/**
 * STUDENT NAME:
 * STUDENT NUMBER:
 * STUDENT NAME:
 * STUDENT NUMBER:
 */

#include "vector_scene.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "primitive_internal.h"

// ============================================================================
// CONCRETE STRUCT LAYOUT (OPAQUE ADT)
// ============================================================================

/**
 * @brief Concrete structure definition for VectorScene.
 * Hidden from vector_scene.h to enforce encapsulation.
 */
struct _VectorScene {
  int width;              /**< Canvas width in pixels */
  int height;             /**< Canvas height in pixels */
  Color bg_color;         /**< Default background color */
  Primitive** primitives; /**< Dynamic array of pointers to Primitive */
  size_t count;           /**< Current number of primitives in scene */
  size_t capacity;        /**< Allocated capacity for primitives array */
  int next_auto_id;       /**< Auto-increment ID generator */
};

#define INITIAL_SCENE_CAPACITY 8

// ============================================================================
// HELPER FUNCTIONS (PROVIDED TO STUDENTS)
// ============================================================================

/**
 * @brief Helper to write an RGB pixel buffer to binary PPM file (P6 format).
 */
static bool save_ppm_image(const char* filename, int width, int height,
                           const uint8_t* pixel_buffer) {
  if (!filename || !pixel_buffer || width <= 0 || height <= 0) {
    return false;
  }

  FILE* file = fopen(filename, "wb");
  if (!file) {
    return false;
  }

  fprintf(file, "P6\n%d %d\n255\n", width, height);

  size_t total_bytes = (size_t)width * height * 3;
  size_t written = fwrite(pixel_buffer, 1, total_bytes, file);
  fclose(file);

  return written == total_bytes;
}

// ============================================================================
// ACCESSORS / GETTERS (PROVIDED TO STUDENTS)
// ============================================================================

int vector_scene_get_width(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->width;
}

int vector_scene_get_height(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->height;
}

Color vector_scene_get_background_color(const VectorScene* scene) {
  if (!scene) {
    Color black = {0, 0, 0};
    return black;
  }
  return scene->bg_color;
}

size_t vector_scene_get_count(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->count;
}

Primitive* vector_scene_get_primitive_by_index(const VectorScene* scene,
                                               size_t index) {
  if (!scene || index >= scene->count) {
    return NULL;
  }
  return scene->primitives[index];
}

Primitive* vector_scene_get_primitive_by_id(const VectorScene* scene, int id) {
  if (!scene || id < 0) {
    return NULL;
  }

  for (size_t i = 0; i < scene->count; i++) {
    if (scene->primitives[i] && scene->primitives[i]->id == id) {
      return scene->primitives[i];
    }
  }
  return NULL;
}

// ============================================================================
// SCENE LIFECYCLE (STUDENTS MUST COMPLETE)
// ============================================================================

VectorScene* vector_scene_create(int width, int height, Color bg_color) {
  if (width <= 0 || height <= 0) {
    return NULL;
  }

  VectorScene* scene = malloc(sizeof(VectorScene));
  if (scene == NULL) {
    return NULL;
  }
  scene->primitives = malloc(INITIAL_SCENE_CAPACITY * sizeof(Primitive*));
  if (scene->primitives == NULL) {
    free(scene);
    return NULL;
  }

  scene->width = width;
  scene->height = height;
  scene->bg_color = bg_color;
  scene->count = 0;
  scene->capacity = INITIAL_SCENE_CAPACITY;
  scene->next_auto_id = 1;  // os ids automaticos comecam em 1
  return scene;
}

void vector_scene_destroy(VectorScene* scene) {
  if (!scene) {
    return;
  }

  for (size_t i = 0; i < scene->count; i++) {
    primitive_destroy(scene->primitives[i]);
  }
  free(scene->primitives);
  free(scene);
}

// ============================================================================
// COLLECTION MANAGEMENT (STUDENTS MUST COMPLETE)
// ============================================================================

int vector_scene_add_primitive(VectorScene* scene, Primitive* p) {
  if (!scene || !p) {
    return -1;
  }

  int id = p->id;
  if (id < 0) {
    id = scene->next_auto_id;
    if (id < 0) {
      // Depois de INT_MAX, procura um ID livre sem overflow.
      id = 1;
      while (vector_scene_get_primitive_by_id(scene, id) != NULL) {
        if (id == INT_MAX) {
          return -1;
        }
        id++;
      }
    }
  } else if (scene->next_auto_id < 0 || id < scene->next_auto_id) {
    // IDs acima do maior usado sao novos: a carga sequencial fica O(N).
    if (vector_scene_get_primitive_by_id(scene, id) != NULL) {
      return -1;
    }
  }

  if (scene->count == scene->capacity) {
    if (scene->capacity > SIZE_MAX / sizeof(Primitive*) / 2) {
      return -1;
    }
    size_t nova_capacidade = scene->capacity * 2;
    Primitive** novo =
        realloc(scene->primitives, nova_capacidade * sizeof(Primitive*));
    if (novo == NULL) {
      return -1;
    }
    scene->primitives = novo;
    scene->capacity = nova_capacidade;
  }

  // So transfere a posse e altera o ID depois de garantir capacidade.
  p->id = id;
  if (scene->next_auto_id >= 0 && id >= scene->next_auto_id) {
    scene->next_auto_id = id == INT_MAX ? -1 : id + 1;
  }

  scene->primitives[scene->count] = p;
  scene->count++;
  primitive_update_aabb(p);
  return p->id;
}

bool vector_scene_remove_primitive(VectorScene* scene, int id) {
  if (!scene || id < 0) {
    return false;
  }

  for (size_t i = 0; i < scene->count; i++) {
    if (scene->primitives[i]->id == id) {
      primitive_destroy(scene->primitives[i]);
      for (size_t j = i; j + 1 < scene->count; j++) {
        scene->primitives[j] = scene->primitives[j + 1];
      }
      scene->count--;
      scene->primitives[scene->count] = NULL;
      return true;
    }
  }
  return false;
}

// ============================================================================
// MANIPULATION AND TRANSFORMATIONS (STUDENTS MUST COMPLETE)
// ============================================================================

bool vector_scene_translate_primitive(VectorScene* scene, int id, double dx,
                                      double dy) {
  if (!scene) {
    return false;
  }

  Primitive* p = vector_scene_get_primitive_by_id(scene, id);
  if (p == NULL) {
    return false;
  }
  primitive_translate(p, dx, dy);
  return true;
}

bool vector_scene_set_z_index(VectorScene* scene, int id, int new_z_index) {
  if (!scene) {
    return false;
  }

  Primitive* p = vector_scene_get_primitive_by_id(scene, id);
  if (p == NULL) {
    return false;
  }
  p->z_index = new_z_index;
  return true;
}

// ============================================================================
// SPATIAL SELECTION AND SORTING (STUDENTS MUST COMPLETE & ANALYZE)
// ============================================================================

/**
 * @brief Sorts an array of primitives by z_index using O(N^2) Insertion Sort.
 *
 * Iterative in-place sorting algorithm. Iterates through the primitive array
 * and inserts each element into its correct sorted position based on z_index.
 * Increments comparison and swap/move metrics in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param count Number of elements in the array.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */
static void insertion_sort_by_z(Primitive** arr, size_t count,
                                SortStats* stats) {
  // Com menos de 2 elementos ja esta ordenado
  if (count < 2) {
    return;
  }

  for (size_t i = 1; i < count; i++) {
    Primitive* chave = arr[i];
    size_t j = i;

    // Desloca para a direita os elementos maiores que a chave.
    // j e size_t, por isso o ciclo para em j > 0 (j >= 0 era sempre verdade).
    // Parar com <= mantem a ordenacao estavel.
    while (j > 0) {
      stats->comparisons++;
      if (arr[j - 1]->z_index <= chave->z_index) {
        break;
      }
      arr[j] = arr[j - 1];
      stats->swaps++;
      j--;
    }
    arr[j] = chave;
  }
}

/**
 * @brief Merges two sorted subarrays of primitive pointers by z_index.
 *
 * Helper function for MergeSort that combines [left..mid] and [mid+1..right]
 * using a temporary pointer array. Increments comparisons and swaps/moves
 * counters in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to merge.
 * @param temp_arr Auxiliary temporary array of Primitive pointers.
 * @param left Starting index of the first subarray.
 * @param mid Middle index separating the two subarrays.
 * @param right Ending index of the second subarray.
 * @param stats Pointer to SortStats structure to track metrics (can be NULL).
 */
static void merge_by_z(Primitive** arr, Primitive** temp_arr, size_t left,
                       size_t mid, size_t right, SortStats* stats) {
  size_t i = left;     // posicao na metade esquerda
  size_t j = mid + 1;  // posicao na metade direita
  size_t k = left;     // posicao no array auxiliar

  // Cada copia para o array auxiliar conta como uma troca
  while (i <= mid && j <= right) {
    if (stats) stats->comparisons++;
    // <= tira da esquerda em caso de empate (ordenacao estavel)
    if (arr[i]->z_index <= arr[j]->z_index) {
      temp_arr[k++] = arr[i++];
    } else {
      temp_arr[k++] = arr[j++];
    }
    if (stats) stats->swaps++;
  }

  // Copia o que sobrou de uma das metades
  while (i <= mid) {
    temp_arr[k++] = arr[i++];
    if (stats) stats->swaps++;
  }
  while (j <= right) {
    temp_arr[k++] = arr[j++];
    if (stats) stats->swaps++;
  }

  // Volta a copiar o resultado para o array original (tambem conta)
  for (k = left; k <= right; k++) {
    arr[k] = temp_arr[k];
    if (stats) stats->swaps++;
  }
}

/**
 * @brief Sorts a subarray of primitives by z_index using O(N log N) MergeSort.
 *
 * Stable recursive Divide-and-Conquer algorithm. Splits the array in half,
 * recursively sorts both halves, and invokes merge_by_z() to merge them.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param temp_arr Auxiliary temporary array allocated for merging.
 * @param left Starting index of the subarray to sort.
 * @param right Ending index of the subarray to sort.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */
static void merge_sort_by_z(Primitive** arr, Primitive** temp_arr, size_t left,
                            size_t right, SortStats* stats) {
  // Caso base: 0 ou 1 elemento ja esta ordenado
  if (left >= right) {
    return;
  }

  size_t mid = left + (right - left) / 2;
  merge_sort_by_z(arr, temp_arr, left, mid, stats);
  merge_sort_by_z(arr, temp_arr, mid + 1, right, stats);
  merge_by_z(arr, temp_arr, left, mid, right, stats);
}

void vector_scene_sort_by_z(VectorScene* scene, bool use_fast_sort,
                            SortStats* stats) {
  // Se o utilizador nao quiser as estatisticas, contamos numa variavel local
  SortStats estatisticas_locais = {0, 0};
  if (stats == NULL) {
    stats = &estatisticas_locais;
  }
  stats->comparisons = 0;
  stats->swaps = 0;

  if (scene == NULL || scene->count < 2) {
    return;
  }

  if (use_fast_sort) {
    Primitive** temp = malloc(scene->count * sizeof(Primitive*));
    if (temp != NULL) {
      merge_sort_by_z(scene->primitives, temp, 0, scene->count - 1, stats);
      free(temp);
      return;
    }
    // Sem memoria para o array auxiliar: usa o insertion sort, que e in-place
  }
  insertion_sort_by_z(scene->primitives, scene->count, stats);
}

// Auxiliar: o ponto esta dentro da caixa delimitadora (fronteira incluida)?
static bool is_point_in_aabb(BoundingBox caixa, Point pt) {
  return pt.x >= caixa.min.x && pt.x <= caixa.max.x && pt.y >= caixa.min.y &&
         pt.y <= caixa.max.y;
}

Primitive* vector_scene_pick_top_primitive(const VectorScene* scene, Point pt,
                                           bool is_opt, PickStats* stats) {
  if (stats != NULL) {
    stats->aabb_tests = 0;
    stats->exact_tests = 0;
  }
  if (!scene) {
    return NULL;
  }

  // A forma mais acima encontrada ate agora (a de maior z_index)
  Primitive* topo = NULL;

  // Percorre de tras para a frente. Com z_index iguais, a que esta mais a
  // direita no array foi pintada depois, logo esta por cima.
  for (size_t i = scene->count; i > 0; i--) {
    Primitive* p = scene->primitives[i - 1];

    // Se ja temos uma forma com z_index maior ou igual, esta fica por baixo
    // e nem vale a pena testa-la
    if (topo != NULL && p->z_index <= topo->z_index) {
      continue;
    }

    // 1.a fase: teste barato a AABB (so na estrategia otimizada)
    if (is_opt) {
      if (stats) stats->aabb_tests++;
      if (!is_point_in_aabb(primitive_get_aabb(p), pt)) {
        continue;
      }
    }

    // 2.a fase: teste exato
    if (stats) stats->exact_tests++;
    if (primitive_contains_point(p, pt)) {
      topo = p;
    }
  }

  return topo;
}

// ============================================================================
// RENDERING / RASTERIZATION (STUDENTS MUST COMPLETE)
// ============================================================================

// Auxiliar: pinta uma forma no buffer de pixeis, so dentro da sua AABB
// (cortada ao tamanho da imagem), fazendo alpha blending em cada pixel.
static void draw_primitive(uint8_t* pixels, int w, int h, Primitive* p) {
  BoundingBox caixa = primitive_get_aabb(p);

  int x0 = (int)fmax(0, floor(caixa.min.x));
  int y0 = (int)fmax(0, floor(caixa.min.y));
  int x1 = (int)fmin(w - 1, ceil(caixa.max.x));
  int y1 = (int)fmin(h - 1, ceil(caixa.max.y));

  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {
      // Testa o centro do pixel
      Point centro = {x + 0.5, y + 0.5};
      if (!primitive_contains_point(p, centro)) {
        continue;
      }

      // cor_final = alpha * cor_nova + (1 - alpha) * cor_fundo
      uint8_t* px = &pixels[3 * ((size_t)y * w + x)];
      px[0] = (uint8_t)(p->alpha * p->color.r + (1.0 - p->alpha) * px[0]);
      px[1] = (uint8_t)(p->alpha * p->color.g + (1.0 - p->alpha) * px[1]);
      px[2] = (uint8_t)(p->alpha * p->color.b + (1.0 - p->alpha) * px[2]);
    }
  }
}

bool vector_scene_render(const VectorScene* scene,
                         const char* output_filename) {
  if (!scene || !output_filename) {
    return false;
  }

  int w = scene->width;
  int h = scene->height;

  uint8_t* pixels = malloc((size_t)w * h * 3);
  if (pixels == NULL) {
    return false;
  }
  for (size_t i = 0; i < (size_t)w * h; i++) {
    pixels[3 * i + 0] = scene->bg_color.r;
    pixels[3 * i + 1] = scene->bg_color.g;
    pixels[3 * i + 2] = scene->bg_color.b;
  }

  // A cena e const, por isso ordenamos uma copia do array de ponteiros.
  // O malloc e de pelo menos 1 elemento porque malloc(0) pode devolver NULL.
  size_t n = scene->count;
  Primitive** ordem = malloc((n > 0 ? n : 1) * sizeof(Primitive*));
  Primitive** temp = malloc((n > 0 ? n : 1) * sizeof(Primitive*));
  if (ordem == NULL || temp == NULL) {
    free(ordem);
    free(temp);
    free(pixels);
    return false;
  }
  memcpy(ordem, scene->primitives, n * sizeof(Primitive*));
  if (n > 1) {
    merge_sort_by_z(ordem, temp, 0, n - 1, NULL);
  }
  free(temp);

  // Pinta da camada mais baixa para a mais alta (algoritmo do pintor)
  for (size_t k = 0; k < n; k++) {
    draw_primitive(pixels, w, h, ordem[k]);
  }
  free(ordem);

  bool ok = save_ppm_image(output_filename, w, h, pixels);
  free(pixels);
  return ok;
}

// ============================================================================
// FILE INPUT / OUTPUT (PROVIDED TO STUDENTS)
// ============================================================================

bool vector_scene_save_text(const VectorScene* scene, const char* filename) {
  if (!scene || !filename) {
    return false;
  }

  FILE* file = fopen(filename, "w");
  if (!file) {
    return false;
  }

  Color bg = scene->bg_color;
  fprintf(file, "SCENE %d %d %d %d %d\n", scene->width, scene->height, bg.r,
          bg.g, bg.b);

  for (size_t i = 0; i < scene->count; i++) {
    Primitive* p = scene->primitives[i];
    if (!p) {
      continue;
    }

    Color c = p->color;

    switch (p->type) {
      case PRIM_CIRCLE:
        fprintf(file, "CIRC %d %d %d %d %d %.3f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha, p->geom.circle.center.x,
                p->geom.circle.center.y, p->geom.circle.radius);
        break;

      case PRIM_RECTANGLE:
        fprintf(file, "RECT %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha,
                p->geom.rectangle.top_left.x, p->geom.rectangle.top_left.y,
                p->geom.rectangle.width, p->geom.rectangle.height);
        break;

      case PRIM_SEGMENT:
        fprintf(file, "SEG %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha, p->geom.segment.p1.x,
                p->geom.segment.p1.y, p->geom.segment.p2.x,
                p->geom.segment.p2.y);
        break;

      case PRIM_POLYGON:
        fprintf(file, "POLY %d %d %d %d %d %.3f %zu", p->id, p->z_index, c.r,
                c.g, c.b, p->alpha, p->geom.polygon.num_vertices);

        for (size_t v = 0; v < p->geom.polygon.num_vertices; v++) {
          fprintf(file, " %.2f %.2f", p->geom.polygon.vertices[v].x,
                  p->geom.polygon.vertices[v].y);
        }
        fprintf(file, "\n");
        break;
    }
  }

  fclose(file);
  return true;
}

VectorScene* vector_scene_load_text(const char* filename) {
  if (!filename) {
    return NULL;
  }

  FILE* file = fopen(filename, "r");
  if (!file) {
    return NULL;
  }

  char line[1024];
  VectorScene* scene = NULL;

  while (fgets(line, sizeof(line), file)) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
      continue;
    }

    char tag[16];
    if (sscanf(line, "%15s", tag) != 1) {
      continue;
    }

    if (strcmp(tag, "SCENE") == 0) {
      int w, h, r, g, b;
      if (sscanf(line, "SCENE %d %d %d %d %d", &w, &h, &r, &g, &b) == 5) {
        Color bg = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
        if (scene != NULL) {
          goto erro_leitura;
        }
        scene = vector_scene_create(w, h, bg);
        if (scene == NULL) {
          goto erro_leitura;
        }
      }
    } else if (scene != NULL) {
      int id, z_index, r, g, b;
      double alpha;

      if (strcmp(tag, "CIRC") == 0) {
        double cx, cy, radius;
        if (sscanf(line, "CIRC %d %d %d %d %d %lf %lf %lf %lf", &id, &z_index,
                   &r, &g, &b, &alpha, &cx, &cy, &radius) == 9) {
          Point center = {cx, cy};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_circle(id, center, radius, color,
                                                 alpha, z_index);
          if (p == NULL) {
            goto erro_leitura;
          }
          if (vector_scene_add_primitive(scene, p) < 0) {
            primitive_destroy(p);
            goto erro_leitura;
          }
        }
      } else if (strcmp(tag, "RECT") == 0) {
        double x, y, w, h;
        if (sscanf(line, "RECT %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x, &y, &w, &h) == 10) {
          Point top_left = {x, y};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_rectangle(id, top_left, w, h, color,
                                                    alpha, z_index);
          if (p == NULL) {
            goto erro_leitura;
          }
          if (vector_scene_add_primitive(scene, p) < 0) {
            primitive_destroy(p);
            goto erro_leitura;
          }
        }
      } else if (strcmp(tag, "SEG") == 0) {
        double x1, y1, x2, y2;
        if (sscanf(line, "SEG %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x1, &y1, &x2, &y2) == 10) {
          Point p1 = {x1, y1};
          Point p2 = {x2, y2};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p =
              primitive_create_segment(id, p1, p2, color, alpha, z_index);
          if (p == NULL) {
            goto erro_leitura;
          }
          if (vector_scene_add_primitive(scene, p) < 0) {
            primitive_destroy(p);
            goto erro_leitura;
          }
        }
      } else if (strcmp(tag, "POLY") == 0) {
        size_t num_verts;
        int read_header = sscanf(line, "POLY %d %d %d %d %d %lf %zu", &id,
                                 &z_index, &r, &g, &b, &alpha, &num_verts);
        if (read_header == 7) {
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_polygon(id, color, alpha, z_index);
          if (p == NULL) {
            goto erro_leitura;
          }
          {
            // Salta a etiqueta e os 7 campos do cabecalho (id, z, r, g, b,
            // alpha, num_verts) para chegar ao primeiro vertice
            char* ptr = line;
            for (int k = 0; k < 8; k++) {
              while (*ptr && *ptr != ' ' && *ptr != '\t') {
                ptr++;
              }
              while (*ptr && (*ptr == ' ' || *ptr == '\t')) {
                ptr++;
              }
            }

            for (size_t v = 0; v < num_verts; v++) {
              double vx, vy;
              if (sscanf(ptr, "%lf %lf", &vx, &vy) == 2) {
                Point pt = {vx, vy};
                if (!polygon_add_vertex(p, pt)) {
                  primitive_destroy(p);
                  goto erro_leitura;
                }

                for (int k = 0; k < 2; k++) {
                  while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' &&
                         *ptr != '\r') {
                    ptr++;
                  }
                  while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' ||
                                  *ptr == '\r')) {
                    ptr++;
                  }
                }
              } else {
                primitive_destroy(p);
                goto erro_leitura;
              }
            }
            if (vector_scene_add_primitive(scene, p) < 0) {
              primitive_destroy(p);
              goto erro_leitura;
            }
          }
        }
      }
    }
  }

  fclose(file);
  return scene;

erro_leitura:
  fclose(file);
  vector_scene_destroy(scene);
  return NULL;
}
