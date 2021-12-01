#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "../lib/term.h"

#define CANVAS_WIDTH 13
#define CANVAS_HEIGHT 24
#define FRAME_TIME 50000

#define TICKS_TO_DROP 10
#define BOARD_WIDTH 10
#define VISIBLE_BOARD_HEIGHT 20
#define BOARD_HEIGHT 24
#define PIECE_SIZE 4

// enum piece
// {
//     PIECE_O,
//     PIECE_S,
//     PIECE_Z,
//     PIECE_I,
//     PIECE_L,
//     PIECE_J,
//     PIECE_T,
// };

struct pos
{
    int x;
    int y;
};

struct piece
{
    char data[PIECE_SIZE][PIECE_SIZE];
};

static const struct piece O_PIECE = {
    .data = {
        {1, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
};

int main(void)
{
    term_init();

    enum term_input input;
    char canvas[CANVAS_HEIGHT * CANVAS_WIDTH];

    char board[BOARD_HEIGHT][BOARD_WIDTH];
    struct piece piece = O_PIECE;
    struct pos piecePos = {.x = 0, .y = 0};

    int tickNum = 0;
    while (true)
    {
        input = term_getInput();

        struct pos wishPiecePos = piecePos;

        if (tickNum % 10 == 0)
        {
            ++wishPiecePos.y;
        }

        switch (input)
        {
        case TERM_ESCAPE:
            exit(0);
        default:
            break;
        }

        //
        // render
        //
        char *pCanvas = canvas;
        for (int i = 0; i < BOARD_WIDTH + 2; ++i)
        {
            *pCanvas++ = '+';
        }
        *pCanvas++ = '\n';

        for (int i = 0; i < VISIBLE_BOARD_HEIGHT; ++i)
        {
            *pCanvas++ = '+';
            for (int j = 0; j < BOARD_WIDTH; ++j)
            {
                *pCanvas++ = ' ';
            }
            *pCanvas++ = '+';
            *pCanvas++ = '\n';
        }

        for (int i = 0; i < BOARD_WIDTH + 2; ++i)
        {
            *pCanvas++ = '+';
        }

        // render piece
        for (int i = 0; i < PIECE_SIZE; ++i)
        {
            for (int j = 0; j < PIECE_SIZE; ++j)
            {
                if (piece.data[i][j])
                {
                    canvas[(piecePos.y + 1 + i) * CANVAS_WIDTH + (piecePos.x + 1 + j)] = '#';
                }
            }
        }

        term_render(canvas, pCanvas - canvas);
        usleep(FRAME_TIME);
        ++tickNum;
    }

    return 0;
}