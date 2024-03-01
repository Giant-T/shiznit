#include <Windows.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define CIRCLE_RADIUS 20
#define PI 3.14159265

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

void display(COORD screen_size) {
    short middle_x = screen_size.X / 2 - 1;
    short middle_y = screen_size.Y / 2 - 1;

    for (short y = middle_y - CIRCLE_RADIUS; y < middle_y + CIRCLE_RADIUS; y++) {
        for (short x = middle_x - CIRCLE_RADIUS; x < middle_x + CIRCLE_RADIUS; x++) {
            short x_diff = abs(x - middle_x);
            short y_diff = abs(y - middle_y);
            short distance = sqrt(x_diff * x_diff + y_diff * y_diff);
            if (distance < CIRCLE_RADIUS) {
                printf("\x1b[%d;%dH#", y, x);
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

    while (1) {
        display(size);
        printf("\x1b[1;1H%d, size: %d, %d", counter++, size.X, size.Y);
        Sleep(500);
    }
}
