#include <Windows.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265

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
    vector_t direction;
    double distance;
} camera_t;

unsigned long out_mode = 0;
HANDLE out_handle;
unsigned long in_mode = 0;
HANDLE in_handle;

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

double ray_distance(const vector_t *ray, const vector_t *vec) {
    vector_t product = cross_product(ray, vec);
    double numerator = vector_len(&product);
    double denumerator = vector_len(vec);

    return numerator / denumerator;
}

void display(COORD *screen_size, const camera_t *camera, const sphere_t *sphere) {
    for (int screen_y = 1; screen_y < screen_size->Y; screen_y++) {
        for (int screen_x = 1; screen_x < screen_size->X; screen_x++) {
            vector_t ray = {
                .x = screen_x - (int)(screen_size->X / 2),
                .y = camera->distance,
                .z = screen_y - (int)(screen_size->Y / 2),
            };

            double distance = ray_distance(&ray, &sphere->pos);

            if (distance < sphere->radius) {
                printf("\x1b[%d;%dH#", screen_y, screen_x);
            }
        }
    }
}

int main(void) {
    boolean initialised = initialise_vt();

    if (!initialised) return 1;

    signal(SIGINT, clean_up);

    CONSOLE_SCREEN_BUFFER_INFO screen_buffer_info;
    GetConsoleScreenBufferInfo(out_handle, &screen_buffer_info);

    COORD size = screen_buffer_info.dwSize;
    int counter = 0;

    const sphere_t sphere = {
        .pos = {
            .x = 0,
            .z = 0,
            .y = 10,
        },
        .radius = 20,
    };

    const camera_t camera = {
        .pos = {
            .x = 0,
            .y = 0,
            .z = 0,
        },
        .distance = 1,
    };

    while (1) {
        display(&size, &camera, &sphere);
        printf("\x1b[1;1H%d, size: %d, %d", counter++, size.X, size.Y);
        Sleep(500);
    }
}
