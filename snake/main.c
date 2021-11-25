#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "term.h"

#define MAP_WIDTH 16
#define MAP_HEIGHT 8
#define CANVAS_WIDTH 19
#define CANVAS_HEIGHT 11
#define FRAME_TIME 50000

struct pos
{
    int x;
    int y;
};

enum direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

enum mapTile
{
    TILE_EMPTY,
    TILE_SNAKE,
    TILE_FOOD,
};

enum gameState
{
    STATE_PLAYING,
    STATE_GAME_OVER,
};

enum term_input input;
char canvas[CANVAS_HEIGHT * CANVAS_WIDTH];
enum gameState state;
int score;
struct pos snake[MAP_HEIGHT * MAP_WIDTH];
int snakeLen;
enum direction dir;
struct pos food;
enum mapTile map[MAP_HEIGHT * MAP_WIDTH];

static char *string_write(char *out, char *str)
{
    char *pStr = str;
    char *pOut = out;
    do
    {
        *pOut++ = *pStr++;
    } while (*pStr != '\0');

    return pOut;
}

static int mod(int a, int b)
{
    return (b + (a % b)) % b;
}

static int randRange(int upper)
{
    return ((float)rand() / RAND_MAX) * upper;
}

static struct pos getFoodSpawn(enum mapTile *map)
{
    struct pos emptySpaces[MAP_HEIGHT * MAP_WIDTH];
    int numEmptySpaces = 0;

    for (int i = 0; i < MAP_HEIGHT; ++i)
    {
        for (int j = 0; j < MAP_WIDTH; ++j)
        {
            if (map[i * MAP_WIDTH + j] == TILE_EMPTY)
            {
                struct pos p = {.x = j, .y = i};
                emptySpaces[numEmptySpaces++] = p;
            }
        }
    }

    return emptySpaces[randRange(numEmptySpaces)];
}

static void createMap(enum mapTile *map, struct pos *snake, int snakeLen)
{
    for (int i = 0; i < MAP_HEIGHT; ++i)
    {
        for (int j = 0; j < MAP_WIDTH; ++j)
        {
            map[i * MAP_WIDTH + j] = TILE_EMPTY;
        }
    }
    for (int i = 0; i < snakeLen; ++i)
    {
        map[snake[i].y * MAP_WIDTH + snake[i].x] = TILE_SNAKE;
    }
}

int main(void)
{
    term_init();

game_start:
    state = STATE_PLAYING;
    score = 0;
    dir = LEFT;

    snake[0].x = MAP_WIDTH / 2;
    snake[0].y = MAP_HEIGHT / 2;
    snake[1].x = MAP_WIDTH / 2 + 1;
    snake[1].y = MAP_HEIGHT / 2;
    snake[2].x = MAP_WIDTH / 2 + 2;
    snake[2].y = MAP_HEIGHT / 2;
    snake[3].x = MAP_WIDTH / 2 + 3;
    snake[3].y = MAP_HEIGHT / 2;
    snakeLen = 4;

    createMap(map, snake, snakeLen);
    food = getFoodSpawn(map);
    map[food.y * MAP_WIDTH + food.x] = TILE_FOOD;

    while (true)
    {
    game_loop:

        input = term_getInput();

        if (state == STATE_PLAYING)
        {
            //
            // handle input
            //
            switch (input)
            {
            case TERM_ESCAPE:
                goto exit;
            case TERM_UP:
                if (dir != DOWN)
                {
                    dir = UP;
                }
                break;
            case TERM_RIGHT:
                if (dir != LEFT)
                {
                    dir = RIGHT;
                }
                break;
            case TERM_DOWN:
                if (dir != UP)
                {
                    dir = DOWN;
                }
                break;
            case TERM_LEFT:
                if (dir != RIGHT)
                {
                    dir = LEFT;
                }
                break;
            default:
                break;
            }

            //
            // update game
            //
            struct pos newPos = snake[0];
            switch (dir)
            {
            case UP:
                newPos.y = mod(newPos.y - 1, MAP_HEIGHT);
                break;
            case RIGHT:
                newPos.x = mod(newPos.x + 1, MAP_WIDTH);
                break;
            case DOWN:
                newPos.y = mod(newPos.y + 1, MAP_HEIGHT);
                break;
            case LEFT:
                newPos.x = mod(newPos.x - 1, MAP_WIDTH);
                break;
            }

            // if we are going to run into ourselves (don't check the last segment, as it will move out of the way)
            for (int i = 0; i < snakeLen - 1; ++i)
            {
                if (newPos.x == snake[i].x && newPos.y == snake[i].y)
                {
                    state = STATE_GAME_OVER;
                    goto game_loop;
                }
            }

            bool foodEaten = false;
            if (newPos.x == food.x && newPos.y == food.y)
            {
                ++snakeLen;
                score += 1;
                if (snakeLen >= MAP_HEIGHT * MAP_WIDTH - 1)
                {
                    state = STATE_GAME_OVER;
                    goto game_loop;
                }
                foodEaten = true;
            }

            // move snake
            for (int i = snakeLen - 1; i >= 1; --i)
            {
                snake[i] = snake[i - 1];
            }
            snake[0] = newPos;

            // create map
            createMap(map, snake, snakeLen);
            if (foodEaten)
            {
                food = getFoodSpawn(map);
            }
            map[food.y * MAP_WIDTH + food.x] = TILE_FOOD;
        }
        else if (state == STATE_GAME_OVER)
        {
            // handle input
            switch (input)
            {
            case TERM_ESCAPE:
                goto exit;
            case TERM_ENTER:
                goto game_start;
            default:
                break;
            }
        }

        //
        // Render
        //
        char *pCanvas = canvas;
        char scoreStr[CANVAS_WIDTH];
        sprintf(scoreStr, "score: %d\n", score);
        pCanvas = string_write(pCanvas, scoreStr);

        for (int i = 0; i < MAP_WIDTH + 2; ++i)
        {
            pCanvas = string_write(pCanvas, "+");
        }

        pCanvas = string_write(pCanvas, "\n");

        for (int i = 0; i < MAP_HEIGHT; ++i)
        {
            pCanvas = string_write(pCanvas, "+");

            for (int j = 0; j < MAP_WIDTH; ++j)
            {
                switch (map[i * MAP_WIDTH + j])
                {
                case TILE_EMPTY:
                    *pCanvas = ' ';
                    break;
                case TILE_SNAKE:
                    *pCanvas = '#';
                    break;
                case TILE_FOOD:
                    *pCanvas = '*';
                    break;
                }
                ++pCanvas;
            }

            pCanvas = string_write(pCanvas, "+\n");
        }

        for (int i = 0; i < MAP_WIDTH + 2; ++i)
        {
            pCanvas = string_write(pCanvas, "+");
        }

        term_render(canvas, pCanvas - canvas);
        usleep(FRAME_TIME);
    }

exit:
    return 0;
}