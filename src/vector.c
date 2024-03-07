#include "vector.h"

vector_t vec_from_points(const vector_t *p1, const vector_t *p2) {
    vector_t vec = {
        .x = p2->x - p1->x,
        .y = p2->y - p1->y,
        .z = p2->z - p1->z,
    };

    return vec;
}

vector_t cross_product(const vector_t *vec1, const vector_t *vec2) {
    vector_t product = {
        .x = (vec1->y * vec2->z) - (vec2->y * vec1->z),
        .y = (vec1->x * vec2->z) - (vec2->x * vec1->z),
        .z = (vec1->x * vec2->y) - (vec2->x * vec1->y),
    };

    return product;
}

vector_t vec_project(const vector_t *vec, const vector_t *onto) {
    double multiplier = dot_product(vec, onto) / dot_product(onto, onto);
    vector_t projection = {
        .x = onto->x * multiplier,
        .y = onto->y * multiplier,
        .z = onto->z * multiplier,
    };

    return projection;
}

vector_t scalar_multiplication(const vector_t *vec, const double scalar) {
    vector_t result = {
        .x = vec->x * scalar,
        .y = vec->y * scalar,
        .z = vec->z * scalar,
    };

    return result;
}

vector_t vec_add(const vector_t *vec1, const vector_t *vec2) {
    vector_t result = {
        .x = vec1->x + vec2->x,
        .y = vec1->y + vec2->y,
        .z = vec1->z + vec2->z,
    };

    return result;
}

vector_t normalize(const vector_t *vec) {
    double len = vector_len(vec);
    vector_t normal = {
        .x = vec->x / len,
        .y = vec->y / len,
        .z = vec->z / len,
    };

    return normal;
}

double line_minimum_distance(const vector_t *vec, const vector_t *point) {
    vector_t product = cross_product(vec, point);
    double numerator = vector_len(&product);
    double denumerator = vector_len(vec);

    return numerator / denumerator;
}

double cosine_similarity(const vector_t *reference, const vector_t *normal) {
    double numerator = dot_product(reference, normal);
    double denumerator = vector_len(reference) * vector_len(normal);
    return -numerator / denumerator;
}
