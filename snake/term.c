#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include "term.h"

// 1MB
#define TERM_BUFFER_SIZE 1024 * 1024
#define TERM_INPUT_TIMEOUT 1

struct term
{
    struct termios defaultTermios;
    char *cursor;
    char *buffer;
};

static struct term gTerm;

static void term_handleError(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}

static void term_destroy(void)
{
    // restore initial terminal settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &gTerm.defaultTermios) == -1)
    {
        term_handleError("tcsetattr");
    }
    free(gTerm.buffer);
}

void term_write(char *s)
{
    char *p = s;
    do
    {
        *gTerm.cursor++ = *p++;
    } while (*p != '\0');
}

void term_render(void)
{
    term_write("\x1b[?25h"); // show cursor
    write(STDOUT_FILENO, gTerm.buffer, gTerm.cursor - gTerm.buffer);
    gTerm.cursor = gTerm.buffer;

    // queue up commands for next render
    term_write("\x1b[?25l"); // hide cursor
    term_write("\x1b[2J");   // clear screen
    term_write("\x1b[H");    // move cursor to start
}

enum term_input term_getInput()
{
    // check until we get a 1 size input
    char c;
    while (read(STDIN_FILENO, &c, 1) != 1)
    {
        // read until we get an input
    }

    // if char was a control char, read the next few
    if (c == '\x1b')
    {
        char sequence[3];
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

    gTerm.buffer = malloc(sizeof(*gTerm.buffer) * TERM_BUFFER_SIZE);
    if (gTerm.buffer == NULL)
    {
        term_handleError("malloc");
    }
    gTerm.cursor = gTerm.buffer;
    term_render();

    atexit(term_destroy);
}
