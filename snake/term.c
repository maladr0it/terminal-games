#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include "term.h"

// 1MB
#define TERM_BUFFER_SIZE 1024 * 1024
#define TERM_INPUT_TIMEOUT 0

struct term
{
    struct termios defaultTermios;
};

static struct term gTerm;

static void term_handleError(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);   // clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);    // move cursor to start
    write(STDOUT_FILENO, "\x1b[?25h", 6); // show cursor
    perror(s);
    exit(1);
}

static void term_destroy(void)
{

    write(STDOUT_FILENO, "\x1b[2J", 4);   // clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);    // move cursor to start
    write(STDOUT_FILENO, "\x1b[?25h", 6); // show cursor
    // restore initial terminal settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &gTerm.defaultTermios) == -1)
    {
        term_handleError("tcsetattr");
    }
}

void term_render(char *str, int len)
{
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);  // move cursor to start
    write(STDOUT_FILENO, str, sizeof(char) * len);
}

enum term_input term_getInput()
{
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1)
    {
        return TERM_NONE;
    }

    // if char was a control char, read the next few
    if (c == '\x1b')
    {
        char sequence[2];
        if (read(STDIN_FILENO, &sequence[0], 1) != 1)
        {
            return TERM_ESCAPE;
        }
        if (read(STDIN_FILENO, &sequence[1], 1) != 1)
        {
            return TERM_ESCAPE;
        }

        if (sequence[0] == '[')
        {
            switch (sequence[1])
            {
            case 'A':
                return TERM_UP;
            case 'B':
                return TERM_DOWN;
            case 'C':
                return TERM_RIGHT;
            case 'D':
                return TERM_LEFT;
            default:
                return TERM_UNRECOGNIZED;
            }
        }

        return TERM_UNRECOGNIZED;
    }

    switch (c)
    {
    case '\r':
        return TERM_ENTER;
    }

    return TERM_UNRECOGNIZED;
}

void term_init(void)
{
    // enable terminal raw mode
    if (tcgetattr(STDIN_FILENO, &gTerm.defaultTermios) == -1)
    {
        term_handleError("tcsgetattr");
    }

    struct termios raw = gTerm.defaultTermios;
    raw.c_iflag &= ~(ICRNL | IXON);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = TERM_INPUT_TIMEOUT;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        term_handleError("tcsetattr");
    }

    write(STDOUT_FILENO, "\x1b[?25l", 6); // hide cursor

    atexit(term_destroy);
}
