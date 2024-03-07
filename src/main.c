#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "terminal.h"
#include "vector.h"

#define F_THRESHOLD 0.000001

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
            const double distance = line_minimum_distance(&ray, &vec);

            double diff = distance - sphere->radius;
            char command[30];
            if (diff < F_THRESHOLD) {
                const double x = sqrt(sphere->radius * sphere->radius - distance * distance);

                const vector_t projection = vec_project(&vec, &ray);
                const vector_t x_proj = scalar_multiplication(&projection, -x / vector_len(&projection));

                vector_t contact = vec_add(&projection, &x_proj);
                vector_t normal = vec_from_points(&contact, &vec);
                normal = normalize(&normal);

                char texture = cosine_similarity(&sun, &normal) > 0 ? '#' : '\'';

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
