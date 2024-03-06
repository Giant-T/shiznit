#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdbool.h>

typedef struct {
    short x, y;
} term_size_t;

bool initialize_terminal(void);
term_size_t get_term_size(void);
void clean_up(int signal);
void wait(int millisec);

#endif  // TERMINAL_H_
