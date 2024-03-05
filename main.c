#include <Windows.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

unsigned long out_mode = 0;
HANDLE out_handle;
unsigned long in_mode = 0;
HANDLE in_handle;

const vector_t sun = {
    .x = -0.588348,
    .y = 0.196116,
    .z = -0.78446,
};

void clean_up(int signal) {
    if (signal == SIGINT) {
        SetConsoleMode(out_handle, out_mode);
        SetConsoleMode(in_handle, in_mode);
        printf("\x1b[?1049l\x1b[?25h");
        exit(0);
    }
}

boolean initialise_vt(void) {
    out_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (out_handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "ERROR: Get stdout failed with %ld\n", GetLastError());
        return false;
    }

    if (!GetConsoleMode(out_handle, &out_mode)) {
        fprintf(stderr, "ERROR: GetConsoleMode of stdout failed with %ld\n", GetLastError());
        return false;
    }

    if (!SetConsoleMode(out_handle, out_mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        fprintf(stderr, "ERROR: SetConsoleMode of stdout failed with %ld\n", GetLastError());
        return false;
    }

    in_handle = GetStdHandle(STD_INPUT_HANDLE);

    if (in_handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "ERROR: Get stdin failed with %ld\n", GetLastError());
        return false;
    }

    if (!GetConsoleMode(in_handle, &in_mode)) {
        fprintf(stderr, "ERROR: GetConsoleMode of stdin failed with %ld\n", GetLastError());
        return false;
    }

    if (!SetConsoleMode(in_handle, in_mode | ENABLE_MOUSE_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT)) {
        fprintf(stderr, "ERROR: SetConsoleMode of stdin failed with %ld\n", GetLastError());
        return false;
    }

    printf("\x1b[?1049h\x1b[?25l");
    return true;
}

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

void display(const COORD *screen_size, const camera_t *camera, const sphere_t *sphere) {
    size_t commands_size = 1024;
    char *commands = malloc(commands_size);
    memset(commands, 0, commands_size);
    strcpy_s(commands, commands_size, "\x1b[2J");
    size_t commands_len = strlen(commands);

    int half_screen_x = (screen_size->X / 2);
    int half_screen_y = (screen_size->Y / 2);

    for (int screen_y = 1; screen_y < screen_size->Y; screen_y++) {
        for (int screen_x = 1; screen_x < screen_size->X; screen_x++) {
            vector_t screen_point = {
                .x = screen_x - half_screen_x,
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
    boolean initialised = initialise_vt();

    if (!initialised) return 1;

    signal(SIGINT, clean_up);

    CONSOLE_SCREEN_BUFFER_INFO screen_buffer_info;
    GetConsoleScreenBufferInfo(out_handle, &screen_buffer_info);

    const COORD *size = &screen_buffer_info.dwSize;
    int counter = 0;

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
        display(size, &camera, &sphere);
        printf("\x1b[1;1H%f, size: %d, %d", camera.pos.y, size->X, size->Y);
        camera.pos.y += 0.25;
        Sleep(500);
    }
}
