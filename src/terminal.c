#include "terminal.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32

#include <Windows.h>

unsigned long out_mode = 0;
HANDLE out_handle;
unsigned long in_mode = 0;
HANDLE in_handle;

bool initialize_terminal() {
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

    signal(SIGINT, clean_up);
    return true;
}

void clean_up(int signal) {
    if (signal == SIGINT) {
        SetConsoleMode(out_handle, out_mode);
        SetConsoleMode(in_handle, in_mode);
        printf("\x1b[?1049l\x1b[?25h");
        exit(0);
    }
}

term_size_t get_term_size() {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(out_handle, &info);

    term_size_t size = {
        .x = info.dwSize.X,
        .y = info.dwSize.Y,
    };

    return size;
}

void wait(int millisec) {
    Sleep(millisec);
}

#elif __linux__

#include <sys/ioctl.h>
#include <unistd.h>

bool initialize_terminal() {
    signal(SIGINT, clean_up);
    return true;
}

void clean_up(int signal) {
    if (signal == SIGINT) {
        printf("\x1b[?1049l\x1b[?25h");
        exit(0);
    }
}

term_size_t get_term_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    term_size_t size = {
        .x = w.ws_row,
        .y = w.ws_col,
    };

    return size;
}

void wait(int millisec) {
    usleep(millisec);
}

#endif
