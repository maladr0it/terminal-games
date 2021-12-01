#ifndef TERMINAL_H
#define TERMINAL_H

enum term_input
{
    TERM_NONE,
    TERM_UNRECOGNIZED,
    TERM_ESCAPE,
    TERM_UP,
    TERM_RIGHT,
    TERM_DOWN,
    TERM_LEFT,
    TERM_ENTER,
};

void term_init(void);

void term_write(char *s);

enum term_input term_getInput();

void term_render(char *str, int len);

#endif