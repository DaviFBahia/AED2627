/**
 * @file primitive.c
 * @brief Implementacao do TAD Primitive (primitivas 2D)
 *
 * Algoritmos e Estruturas de Dados 2026/2027
 * Departamento de Electrónica, Telecomunicações e Informática - UA
 */

/**
 * STUDENT NAME: Davi Ferreira Navarro Bahia
 * STUDENT NUMBER: 132261
 * STUDENT NAME: Sofia Gouveia de Pires Marques
 * STUDENT NUMBER: 132405
 */

#include "primitive.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "primitive_internal.h"

// Um segmento tem espessura zero, por isso um ponto pertence-lhe se estiver
// a esta distancia (em pixeis) ou menos.
#define TOLERANCIA_SEGMENTO 2.0

// ============================================================================
// HELPER GEOMETRIC MATH FUNCTIONS (PROVIDED)
// ============================================================================

/**
 * @brief Tests exact containment of a point inside a circle.
 */
static bool math_circle_contains_point(Point center, double radius, Point pt) {
  double dx = pt.x - center.x;
  double dy = pt.y - center.y;
  return (dx * dx + dy * dy) <= (radius * radius);
}

/**
 * @brief Tests exact containment of a point inside an axis-aligned rectangle.
 */
static bool math_rect_contains_point(Point top_left, double width,
                                     double height, Point pt) {
  return (pt.x >= top_left.x && pt.x <= (top_left.x + width) &&
          pt.y >= top_left.y && pt.y <= (top_left.y + height));
}

/**
 * @brief Tests exact containment of a point near line segment (with tolerance).
 */
static bool math_segment_contains_point(Point p1, Point p2, double tolerance,
                                        Point pt) {
  double l2 = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
  if (l2 == 0.0) {
    double dx = pt.x - p1.x;
    double dy = pt.y - p1.y;
    return sqrt(dx * dx + dy * dy) <= tolerance;
  }
  double t =
      ((pt.x - p1.x) * (p2.x - p1.x) + (pt.y - p1.y) * (p2.y - p1.y)) / l2;
  if (t < 0.0) {
    t = 0.0;
  } else if (t > 1.0) {
    t = 1.0;
  }

  double proj_x = p1.x + t * (p2.x - p1.x);
  double proj_y = p1.y + t * (p2.y - p1.y);
  double dx = pt.x - proj_x;
  double dy = pt.y - proj_y;

  return sqrt(dx * dx + dy * dy) <= tolerance;
}

/**
 * @brief Tests containment of point inside polygon using Ray-Casting.
 */
static bool math_polygon_contains_point(const Point* vertices,
                                        size_t num_vertices, Point pt) {
  if (num_vertices < 3 || vertices == NULL) {
    return false;
  }

  bool inside = false;
  for (size_t i = 0, j = num_vertices - 1; i < num_vertices; j = i++) {
    double xi = vertices[i].x, yi = vertices[i].y;
    double xj = vertices[j].x, yj = vertices[j].y;

    bool intersect = ((yi > pt.y) != (yj > pt.y)) &&
                     (pt.x < (xj - xi) * (pt.y - yi) / (yj - yi) + xi);
    if (intersect) {
      inside = !inside;
    }
  }
  return inside;
}

// ============================================================================
// INTERNAL AUXILIARY FUNCTIONS (PROVIDED)
// ============================================================================

/**
 * @brief Allocates base Primitive structure and initializes common fields.
 */
static Primitive* primitive_allocate_common(int id, PrimitiveType type,
                                            Color color, double alpha,
                                            int z_index) {
  Primitive* p = calloc(1, sizeof(Primitive));
  if (p == NULL) {
    return NULL;
  }

  p->id = id;
  p->type = type;
  p->color = color;
  p->alpha = alpha;
  p->z_index = z_index;
  p->is_aabb_dirty = true;
  p->aabb.min = (Point){0.0, 0.0};
  p->aabb.max = (Point){0.0, 0.0};

  return p;
}

// ============================================================================
// INSPECTION / GETTER FUNCTIONS (PROVIDED TO STUDENTS)
// ============================================================================

int primitive_get_id(const Primitive* p) {
  if (p == NULL) return -1;
  return p->id;
}

PrimitiveType primitive_get_type(const Primitive* p) {
  assert(p != NULL && "Primitive pointer is NULL");

  return p->type;
}

int primitive_get_z_index(const Primitive* p) {
  if (p == NULL) return 0;
  return p->z_index;
}

Color primitive_get_color(const Primitive* p) {
  if (p == NULL) {
    Color black = {0, 0, 0};
    return black;
  }
  return p->color;
}

double primitive_get_alpha(const Primitive* p) {
  if (p == NULL) return 0.0;
  return p->alpha;
}

BoundingBox primitive_get_aabb(Primitive* p) {
  assert(p != NULL && "Primitive pointer is NULL");

  // Force AABB update
  if (p->is_aabb_dirty) {
    primitive_update_aabb(p);
  }
  return p->aabb;
}

size_t polygon_get_num_vertices(const Primitive* p) {
  if (p == NULL || p->type != PRIM_POLYGON) return 0;
  return p->geom.polygon.num_vertices;
}

// ============================================================================
// CREATION AND DESTRUCTION FUNCTIONS
// ============================================================================

/**
 * [PROVIDED AS REFERENCE MODEL]
 * Creates and dynamically allocates a Circle primitive.
 */
Primitive* primitive_create_circle(int id, Point center, double radius,
                                   Color color, double alpha, int z_index) {
  if (radius <= 0.0 || alpha < 0.0 || alpha > 1.0) {
    return NULL;
  }

  Primitive* p =
      primitive_allocate_common(id, PRIM_CIRCLE, color, alpha, z_index);
  if (p == NULL) {
    return NULL;
  }

  p->geom.circle.center = center;
  p->geom.circle.radius = radius;

  // Automatically calculate initial AABB
  primitive_update_aabb(p);

  return p;
}

Primitive* primitive_create_rectangle(int id, Point top_left, double width,
                                      double height, Color color, double alpha,
                                      int z_index) {
  if (width <= 0.0 || height <= 0.0 || alpha < 0.0 || alpha > 1.0) {
    return NULL;
  }

  Primitive* p =
      primitive_allocate_common(id, PRIM_RECTANGLE, color, alpha, z_index);
  if (p == NULL) {
    return NULL;
  }

  p->geom.rectangle.height = height;
  p->geom.rectangle.top_left = top_left;
  p->geom.rectangle.width = width;

  primitive_update_aabb(p);
  return p;
}

Primitive* primitive_create_segment(int id, Point p1, Point p2, Color color,
                                    double alpha, int z_index) {
  if (alpha > 1.0 || alpha < 0.0) {
    return NULL;
  }

  Primitive* p =
      primitive_allocate_common(id, PRIM_SEGMENT, color, alpha, z_index);
  if (p == NULL) {
    return NULL;
  }

  p->geom.segment.p1 = p1;
  p->geom.segment.p2 = p2;

  primitive_update_aabb(p);

  return p;
}

Primitive* primitive_create_polygon(int id, Color color, double alpha,
                                    int z_index) {
  if (alpha < 0.0 || alpha > 1.0) {
    return NULL;
  }
  Primitive* p =
      primitive_allocate_common(id, PRIM_POLYGON, color, alpha, z_index);

  if (p == NULL) {
    return NULL;
  }
  p->geom.polygon.capacity = 0;
  p->geom.polygon.num_vertices = 0;
  p->geom.polygon.vertices = NULL;
  primitive_update_aabb(p);
  return p;
}

void primitive_destroy(Primitive* p) {
  if (p == NULL) {
    return;
  }

  // So o poligono tem memoria propria (o array de vertices)
  if (p->type == PRIM_POLYGON) {
    free(p->geom.polygon.vertices);
  }
  free(p);
}

// ============================================================================
// DYNAMIC ARRAY MANIPULATION (POLYGON)
// ============================================================================

bool polygon_add_vertex(Primitive* p, Point pt) {
  if (p == NULL || p->type != PRIM_POLYGON) {
    return false;
  }

  if (p->geom.polygon.capacity == p->geom.polygon.num_vertices) {
    size_t nova_capacidade;
    if (p->geom.polygon.capacity == 0) {
      nova_capacidade = 4;
    } else {
      nova_capacidade = 2 * p->geom.polygon.capacity;
    }

    // Ponteiro temporario: se o realloc falhar, o array antigo nao se perde
    Point* novo =
        realloc(p->geom.polygon.vertices, nova_capacidade * sizeof(Point));
    if (novo == NULL) {
      return false;
    }
    p->geom.polygon.vertices = novo;
    p->geom.polygon.capacity = nova_capacidade;
  }

  p->geom.polygon.vertices[p->geom.polygon.num_vertices] = pt;
  p->geom.polygon.num_vertices++;

  // A forma mudou, por isso a AABB tem de ser recalculada
  p->is_aabb_dirty = true;
  return true;
}

bool polygon_remove_vertex(Primitive* p, size_t index) {
  // O tipo tem de ser verificado antes de olhar para a union
  if (p == NULL || p->type != PRIM_POLYGON) {
    return false;
  }
  if (index >= p->geom.polygon.num_vertices) {
    return false;
  }

  // Desloca os vertices seguintes uma posicao para a esquerda
  for (size_t i = index; i + 1 < p->geom.polygon.num_vertices; i++) {
    p->geom.polygon.vertices[i] = p->geom.polygon.vertices[i + 1];
  }

  p->geom.polygon.num_vertices--;
  p->is_aabb_dirty = true;
  return true;
}

// ============================================================================
// GEOMETRIC OPERATIONS AND TRANSFORMATIONS
// ============================================================================

void primitive_update_aabb(Primitive* p) {
  if (p == NULL || !p->is_aabb_dirty) {
    return;
  }

  switch (p->type) {
    case PRIM_CIRCLE:
      p->aabb.min.x = p->geom.circle.center.x - p->geom.circle.radius;
      p->aabb.min.y = p->geom.circle.center.y - p->geom.circle.radius;
      p->aabb.max.x = p->geom.circle.center.x + p->geom.circle.radius;
      p->aabb.max.y = p->geom.circle.center.y + p->geom.circle.radius;
      break;

    case PRIM_RECTANGLE:
      p->aabb.min = p->geom.rectangle.top_left;
      p->aabb.max.x = p->aabb.min.x + p->geom.rectangle.width;
      p->aabb.max.y = p->aabb.min.y + p->geom.rectangle.height;
      break;

    case PRIM_SEGMENT:
      p->aabb.min.x = fmin(p->geom.segment.p1.x, p->geom.segment.p2.x)
                      - TOLERANCIA_SEGMENTO;
      p->aabb.min.y = fmin(p->geom.segment.p1.y, p->geom.segment.p2.y)
                      - TOLERANCIA_SEGMENTO;
      p->aabb.max.x = fmax(p->geom.segment.p1.x, p->geom.segment.p2.x)
                      + TOLERANCIA_SEGMENTO;
      p->aabb.max.y = fmax(p->geom.segment.p1.y, p->geom.segment.p2.y)
                      + TOLERANCIA_SEGMENTO;
      break;

    case PRIM_POLYGON:
      // Poligono vazio: AABB na origem.
      p->aabb.min = (Point){0.0, 0.0};
      p->aabb.max = (Point){0.0, 0.0};
      if (p->geom.polygon.num_vertices > 0) {
        p->aabb.min = p->geom.polygon.vertices[0];
        p->aabb.max = p->geom.polygon.vertices[0];
        for (size_t i = 1; i < p->geom.polygon.num_vertices; i++) {
          Point vertice = p->geom.polygon.vertices[i];
          p->aabb.min.x = fmin(p->aabb.min.x, vertice.x);
          p->aabb.min.y = fmin(p->aabb.min.y, vertice.y);
          p->aabb.max.x = fmax(p->aabb.max.x, vertice.x);
          p->aabb.max.y = fmax(p->aabb.max.y, vertice.y);
        }
      }
      break;
  }

  p->is_aabb_dirty = false;
}

void primitive_translate(Primitive* p, double dx, double dy) {
  if (p == NULL) {
    return;
  }

  switch (p->type) {
    case PRIM_CIRCLE:
      p->geom.circle.center.x += dx;
      p->geom.circle.center.y += dy;
      break;
    case PRIM_RECTANGLE:
      p->geom.rectangle.top_left.x += dx;
      p->geom.rectangle.top_left.y += dy;
      break;
    case PRIM_SEGMENT:
      p->geom.segment.p1.x += dx;
      p->geom.segment.p1.y += dy;
      p->geom.segment.p2.x += dx;
      p->geom.segment.p2.y += dy;
      break;
    case PRIM_POLYGON:
      for (size_t i = 0; i < p->geom.polygon.num_vertices; i++) {
        p->geom.polygon.vertices[i].x += dx;
        p->geom.polygon.vertices[i].y += dy;
      }
      break;
  }
  p->is_aabb_dirty = true;
}

// Auxiliar: afasta (ou aproxima) um ponto do centro, multiplicando a
// distancia ao centro por factor.
static Point scale_point_from_center(Point pt, Point centro, double factor) {
  Point novo = {centro.x + (pt.x - centro.x) * factor,
                centro.y + (pt.y - centro.y) * factor};
  return novo;
}

void primitive_scale(Primitive* p, double factor) {
  if (p == NULL || factor <= 0.0) {
    return;
  }

  switch (p->type) {
    case PRIM_CIRCLE:
      p->geom.circle.radius *= factor;
      break;
    case PRIM_RECTANGLE:
      p->geom.rectangle.width *= factor;
      p->geom.rectangle.height *= factor;
      break;
    case PRIM_SEGMENT: {
      Point centro = {(p->geom.segment.p1.x + p->geom.segment.p2.x) / 2.0,
                      (p->geom.segment.p1.y + p->geom.segment.p2.y) / 2.0};
      p->geom.segment.p1 =
          scale_point_from_center(p->geom.segment.p1, centro, factor);
      p->geom.segment.p2 =
          scale_point_from_center(p->geom.segment.p2, centro, factor);
      break;
    }
    case PRIM_POLYGON: {
      // O centro e o da AABB; primitive_get_aabb garante que esta atualizada
      BoundingBox caixa = primitive_get_aabb(p);
      Point centro = {(caixa.min.x + caixa.max.x) / 2.0,
                      (caixa.min.y + caixa.max.y) / 2.0};
      for (size_t i = 0; i < p->geom.polygon.num_vertices; i++) {
        p->geom.polygon.vertices[i] = scale_point_from_center(
            p->geom.polygon.vertices[i], centro, factor);
      }
      break;
    }
  }
  p->is_aabb_dirty = true;
}

bool primitive_contains_point(const Primitive* p, Point pt) {
  if (p == NULL) {
    return false;
  }

  // Narrow-phase exact geometric test dispatch
  switch (p->type) {
    case PRIM_CIRCLE:
      return math_circle_contains_point(p->geom.circle.center,
                                        p->geom.circle.radius, pt);

    case PRIM_RECTANGLE:
      return math_rect_contains_point(p->geom.rectangle.top_left,
                                      p->geom.rectangle.width,
                                      p->geom.rectangle.height, pt);

    case PRIM_SEGMENT:
      return math_segment_contains_point(p->geom.segment.p1, p->geom.segment.p2,
                                         TOLERANCIA_SEGMENTO, pt);

    case PRIM_POLYGON:
      if (p->geom.polygon.num_vertices < 3) {
        return false;
      }
      // O contrato inclui as arestas e vertices, alem do interior.
      for (size_t i = 0, j = p->geom.polygon.num_vertices - 1;
           i < p->geom.polygon.num_vertices; j = i++) {
        if (math_segment_contains_point(p->geom.polygon.vertices[j],
                                        p->geom.polygon.vertices[i], 0.0, pt)) {
          return true;
        }
      }
      return math_polygon_contains_point(p->geom.polygon.vertices,
                                         p->geom.polygon.num_vertices, pt);
  }

  return false;
}
