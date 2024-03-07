#ifndef VECTOR_H_
#define VECTOR_H_

#include <math.h>

typedef struct {
    double x, y, z;
} vector_t;

vector_t vec_from_points(const vector_t *p1, const vector_t *p2);
inline double vector_len(const vector_t *vec);
vector_t cross_product(const vector_t *vec1, const vector_t *vec2);
inline double dot_product(const vector_t *vec1, const vector_t *vec2);
vector_t vec_project(const vector_t *vec, const vector_t *onto);
vector_t scalar_multiplication(const vector_t *vec, const double scalar);
vector_t vec_add(const vector_t *vec1, const vector_t *vec2);
vector_t normalize(const vector_t *vec);
double line_minimum_distance(const vector_t *vec, const vector_t *point);
double cosine_similarity(const vector_t *reference, const vector_t *normal);

inline double vector_len(const vector_t *vec) {
    return sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
}

inline double dot_product(const vector_t *vec1, const vector_t *vec2) {
    return vec1->x * vec2->x + vec1->y * vec2->y + vec1->z * vec2->z;
}

#endif  // VECTOR_H_
