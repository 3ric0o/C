#include "snake_interface.h"
#include <stdlib.h>
#include <raylib.h>

#define MAX_SNAKE_LENGTH 100

typedef struct
{
    Position positions[MAX_SNAKE_LENGTH];
    int length;

} SnakeArray;

static SnakeArray snake_array;
static SnakeInterface snake_interface;

// Implementations of the SnakeInterface functions
static void array_init(int initial_x, int initial_y)
{
    snake_array.length = 1;
    snake_array.positions[0].x = initial_x;
    snake_array.positions[0].y = initial_y;
}

static void array_move(int x, int y)
{
    for(int i = snake_array.length - 1; i > 0; i--)
    {
        snake_array.positions[i] = snake_array.positions[i - 1];
    }
    snake_array.positions[0].x += x;
    snake_array.positions[0].y += y;
}

static void array_grow(void)
{
    if(snake_array.length < MAX_SNAKE_LENGTH)
    {
        snake_array.length++;
        snake_array.positions[snake_array.length - 1] = snake_array.positions[snake_array.length - 2];
    }
}

static bool array_check_collision(Position pos)
{
    for(int i = 0; i < snake_array.length; i++)
    {
        if(snake_array.positions[i].x == pos.x && snake_array.positions[i].y == pos.y)
        {
            return true;
        }
    }
    return false;
}

static Position array_get_head(void)
{
    return snake_array.positions[0];
}

static void array_draw(void)
{
    for(int i = 0; i < snake_array.length; i++)
    {
        DrawRectangle(snake_array.positions[i].x, snake_array.positions[i].y, 10, 10, GREEN);
    }
}

static void array_cleanup(void)
{
    // Nothing to clean up for array implementation
}

// Initialize the snake interface
SnakeInterface *get_snake_array(void)
{
    snake_interface.init = array_init;
    snake_interface.move = array_move;
    snake_interface.grow = array_grow;
    snake_interface.check_collision = array_check_collision;
    snake_interface.get_head = array_get_head;
    snake_interface.draw = array_draw;
    snake_interface.cleanup = array_cleanup;

    return &snake_interface;
}