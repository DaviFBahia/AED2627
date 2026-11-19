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
  scene->next_auto_id = 0;
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
  if (id >= 0) {
    if (vector_scene_get_primitive_by_id(scene, id) != NULL) {
      return -1;
    }
  } else {
    id = scene->next_auto_id;
    if (id < 0 || (id == INT_MAX &&
                  vector_scene_get_primitive_by_id(scene, id) != NULL)) {
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

  // A cena so recebe a posse depois de garantir espaco no array.
  p->id = id;
  scene->primitives[scene->count] = p;
  scene->count++;
  if (scene->next_auto_id >= 0 && id >= scene->next_auto_id) {
    if (id < INT_MAX) {
      scene->next_auto_id = id + 1;
    } else if (scene->next_auto_id == INT_MAX) {
      // Evita overflow; -1 indica que os IDs automaticos se esgotaram.
      scene->next_auto_id = -1;
    }
    // Um INT_MAX explicito nao impede usar IDs automaticos inferiores.
  }
  primitive_update_aabb(p);
  return id;
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

  if ( count < 2) {
    return; // se for menos de 2 elementos, ja está ordenado
  }

  for (size_t i = 1; i <count; i++) {
    Primitive* chave = arr[i];
    size_t j = i;

    while(j > 0) {
      stats->comparisons++;
      if (arr[j-1]->z_index <= chave->z_index) break;
      arr[j] = arr[j-1];
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

  size_t i = left;
  size_t j = mid +1;
  size_t k = left;

  while(i <= mid && j <= right) {
    if (stats) stats->comparisons++; // - Compare z_index of primitives and increment stats->comparisons.
    if (arr[i]->z_index <= arr[j]->z_index) {
      temp_arr[k++] = arr[i++];
    } else {
      temp_arr[k++] = arr[j++];
    }
    if (stats) stats->swaps++;  // - Increment stats->swaps when copying elements to temp_arr and back to arr.
  }

  while ( i<= mid) {
    temp_arr[k++] = arr[i++];
    if(stats) stats->swaps++;
  }
  while (j<= right) {
    temp_arr[k++] = arr[j++];
    if (stats) stats->swaps++;
  }
  
  for (k = left; k <= right; k++) { // - Copy merged elements back from temp_arr to arr[left..right].
    arr[k] = temp_arr[k];
    if(stats) stats->swaps++;
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
  if (left >= right) return; // - Base case: if left >= right, return.
  size_t mid = left + (right - left) / 2; // - Calculate mid = left + (right - left) / 2.
  merge_sort_by_z(arr, temp_arr, left, mid, stats);
  merge_sort_by_z(arr, temp_arr, mid +1, right, stats);
  merge_by_z(arr, temp_arr, left, mid, right, stats);  // - Call merge_by_z to merge the two sorted halves.
}

void vector_scene_sort_by_z(VectorScene* scene, bool use_fast_sort,
                            SortStats* stats) {

  if(stats != NULL) { // Reset stats (comparisons = 0, swaps = 0).
    stats->comparisons = 0;
    stats->swaps = 0;
  }


  if (scene == NULL || scene->count < 2) return;
  
  if (use_fast_sort) { // If use_fast_sort is true: execute O(N log N) sort (MergeSort).
    Primitive** temp = malloc(scene->count * sizeof(Primitive*));
    if(temp != NULL) {
      merge_sort_by_z(scene->primitives, temp, 0, scene->count -1, stats);
      free(temp);
      return;
    }
  }   // If use_fast_sort is false: execute O(N^2) sort (Insertion Sort).
  insertion_sort_by_z(scene->primitives, scene->count, stats);
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

  Primitive* topo = NULL; //a cena mais acima (z_index maior) encontrada até agora

  //percorre de trás para frente, em caso de z_index iguais,
  //a que está mais a direita no array foi pintada depois, logo está por cima
  for(size_t i = scene->count; i >0; i--) {
    Primitive* p = scene->primitives[i-1];
    //se ja existe uma forma com camada maior ou igual, está por baixo
    if (topo != NULL && p->z_index <= topo->z_index) {
      continue;
    }

    if (is_opt) { //  Phase 1 (Broad-phase): AABB check (increment stats->aabb_tests++).
      if (stats) stats->aabb_tests++;
      BoundingBox caixa = primitive_get_aabb(p);
      if (pt.x < caixa.min.x || pt.x > caixa.max.x
        || pt.y < caixa.min.y || pt.y > caixa.max.y) {  // verificar se o ponto está na caixa
        continue;
      }
    }

    // Phase 2 (Narrow-phase): Exact check primitive_contains_point
    if (stats) stats->exact_tests++;
    if(primitive_contains_point(p, pt)) {
      topo = p;
    }
  }

  return topo;
}

// ============================================================================
// RENDERING / RASTERIZATION (STUDENTS MUST COMPLETE)
// ============================================================================

bool vector_scene_render(const VectorScene* scene,
                          const char* output_filename) {
  if (!scene || !output_filename) {
    return false;
  }

  int w = scene->width, h = scene->height;

  uint8_t* pixels = malloc((size_t)w * h * 3);
  if ( pixels == NULL) return false;
  for (size_t i = 0; i <(size_t) w* h; i++) {
    pixels[3 * i + 0] = scene->bg_color.r;
    pixels[3 * i + 1] = scene->bg_color.g;
    pixels[3 * i + 2] = scene->bg_color.b;
  }

  size_t n = scene->count;
  Primitive** ordem = malloc((n > 0 ? n : 1) * sizeof(Primitive*));
  Primitive** temp = malloc((n>0 ? n : 1) * sizeof(Primitive*));
  if (ordem == NULL || temp == NULL) {
    free(ordem); free(temp); free(pixels);
    return false;
  }
  memcpy(ordem, scene->primitives, n * sizeof(Primitive*));
  if (n > 1) merge_sort_by_z(ordem, temp, 0, n -1, NULL);
  free(temp);

  // pintar formas da camada mais baixa para a mais alta
  for (size_t k = 0; k < n; k++) {
    Primitive* p = ordem[k];
    BoundingBox caixa = primitive_get_aabb(p);

    int x0 = (int)fmax(0, floor(caixa.min.x));
    int y0 = (int)fmax(0, floor(caixa.min.y));
    int x1 = (int)fmin(w - 1, ceil(caixa.max.x));
    int y1 = (int)fmin(h - 1, ceil(caixa.max.y));

    for (int y = y0; y <= y1; y++) {
      for (int x = x0; x <= x1; x++) {
        Point centro = {x + 0.5, y + 0.5};
        if(!primitive_contains_point(p, centro)) continue;

        uint8_t* px = &pixels[3 * ((size_t)y * w +x)];

        px[0] = (uint8_t)(p->alpha * p->color.r + (1.0 - p->alpha) * px[0]);
        px[1] = (uint8_t)(p->alpha * p->color.g + (1.0 - p->alpha) * px[1]);
        px[2] = (uint8_t)(p->alpha * p->color.b + (1.0 - p->alpha) * px[2]);
      }
    }
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
        scene = vector_scene_create(w, h, bg);
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
          if (p) {
            vector_scene_add_primitive(scene, p);
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
          if (p) {
            vector_scene_add_primitive(scene, p);
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
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "POLY") == 0) {
        size_t num_verts;
        int read_header = sscanf(line, "POLY %d %d %d %d %d %lf %zu", &id,
                                 &z_index, &r, &g, &b, &alpha, &num_verts);
        if (read_header == 7) {
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_polygon(id, color, alpha, z_index);
          if (p) {
            char* ptr = line;
            for (int k = 0; k < 7; k++) {
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
                polygon_add_vertex(p, pt);

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
              }
            }
            vector_scene_add_primitive(scene, p);
          }
        }
      }
    }
  }

  fclose(file);
  return scene;
}
