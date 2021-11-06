#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>

static struct termios originalTermios;

static void die(const char *s)
{
    perror(s);
    exit(1);
}

static void disableRawMode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios) == -1)
    {
        die("tcsetattr");
    }
}

static void enableRawMode(void)
{
    if (tcgetattr(STDIN_FILENO, &originalTermios) == -1)
    {
        die("tcsgetattr");
    }

    struct termios raw = originalTermios;
    raw.c_iflag &= ~(ICRNL | IXON);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = -1; // wait forever for input

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        die("tcsetattr");
    }
}

int main(void)
{
    enableRawMode();
    atexit(disableRawMode);

    while (1)
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1)
        {
            die("read");
        }

        if (iscntrl(c))
        {
            printf("%d\r\n", c);
        }
        else
        {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q')
        {
            break;
        }
    }

    return 0;
}