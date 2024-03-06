#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal.h"

#define F_THRESHOLD 0.000001

typedef struct {
    double x;
    double y;
    double z;
} vector_t;

typedef struct {
    vector_t pos;
    double radius;
} sphere_t;

typedef struct {
    vector_t pos;
    double v_angle;
    double h_angle;
    double distance;
} camera_t;

const vector_t sun = {
    .x = -0.588348,
    .y = 0.196116,
    .z = -0.78446,
};

vector_t vec_from_points(const vector_t *p1, const vector_t *p2) {
    vector_t vec = {
        .x = p2->x - p1->x,
        .y = p2->y - p1->y,
        .z = p2->z - p1->z,
    };

    return vec;
}

double vector_len(const vector_t *vec) {
    return sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
}

vector_t cross_product(const vector_t *vec1, const vector_t *vec2) {
    vector_t product = {
        .x = (vec1->y * vec2->z) - (vec2->y * vec1->z),
        .y = (vec1->x * vec2->z) - (vec2->x * vec1->z),
        .z = (vec1->x * vec2->y) - (vec2->x * vec1->y),
    };

    return product;
}

double dot_product(const vector_t *vec1, const vector_t *vec2) {
    return vec1->x * vec2->x + vec1->y * vec2->y + vec1->z * vec2->z;
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

double ray_distance(const vector_t *ray, const vector_t *vec) {
    vector_t product = cross_product(ray, vec);
    double numerator = vector_len(&product);
    double denumerator = vector_len(ray);

    return numerator / denumerator;
}

// cosine similarity
double calc_light(const vector_t *normal) {
    double numerator = dot_product(&sun, normal);
    double denumerator = vector_len(&sun) * vector_len(normal);
    return -numerator / denumerator;
}

void display(const term_size_t *screen_size, const camera_t *camera, const sphere_t *sphere) {
    size_t commands_size = 1024;
    char *commands = malloc(commands_size);
    memset(commands, 0, commands_size);
    strcpy_s(commands, commands_size, "\x1b[2J");
    size_t commands_len = strlen(commands);

    int half_screen_x = (screen_size->x / 4);
    int half_screen_y = (screen_size->y / 2);

    for (int screen_y = 1; screen_y < screen_size->y; screen_y++) {
        for (int screen_x = 1; screen_x < screen_size->x; screen_x++) {
            vector_t screen_point = {
                .x = screen_x * 0.5 - half_screen_x,
                .y = camera->pos.y + camera->distance,
                .z = screen_y - half_screen_y,
            };

            const vector_t ray = vec_from_points(&camera->pos, &screen_point);

            const vector_t vec = vec_from_points(&camera->pos, &sphere->pos);
            const double distance = ray_distance(&ray, &vec);

            double diff = distance - sphere->radius;
            char command[20];
            if (diff < F_THRESHOLD) {
                const double x = sqrt(sphere->radius * sphere->radius - distance * distance);

                const vector_t projection = vec_project(&vec, &ray);
                const vector_t x_proj = scalar_multiplication(&projection, -x / vector_len(&projection));

                vector_t contact = vec_add(&projection, &x_proj);
                contact = vec_from_points(&contact, &vec);
                contact = normalize(&contact);

                char texture = calc_light(&contact) > 0 ? '\'' : '#';

                sprintf(command, "\x1b[%d;%dH%c", screen_y, screen_x, texture);
                commands_len += strlen(command);
                if (commands_len >= commands_size) {
                    commands_size *= 2;
                    commands = realloc(commands, commands_size);
                }
                strcat_s(commands, commands_size, command);
            }
        }
    }

    printf("%s", commands);
    free(commands);
}

int main(void) {
    if (!initialize_terminal()) return 1;

    const term_size_t size = get_term_size();

    const sphere_t sphere = {
        .pos = {
            .x = 0,
            .z = 0,
            .y = 30,
        },
        .radius = 20,
    };

    camera_t camera = {
        .pos = {
            .x = 0,
            .y = -5,
            .z = 0,
        },
        .distance = 5,
    };

    while (1) {
        display(&size, &camera, &sphere);
        printf("\x1b[1;1H%f, size: %d, %d", camera.pos.y, size.x, size.y);
        camera.pos.y += 0.25;
        wait(500);
    }
}
