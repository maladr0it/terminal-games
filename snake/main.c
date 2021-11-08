#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "term.h"

int main(void)
{
    term_init();

    bool running = true;

    while (running)
    {
        enum term_input input = term_getInput();

        switch (input)
        {
        case TERM_ESCAPE:
            running = false;
            break;
        case TERM_UP:
            term_write("user pressed up");
            break;
        case TERM_RIGHT:
            term_write("user pressed right");
            break;
        case TERM_DOWN:
            term_write("user pressed down");
            break;
        case TERM_LEFT:
            term_write("user pressed left");
            break;
        default:
            break;
        }

        term_render();
    }

    return 0;
}