#ifndef SNAKE_SNAKE_INTERFACE_H
#define SNAKE_SNAKE_INTERFACE_H

#include <stdbool.h>

typedef struct
{
    int x;
    int y;

} Position;

typedef struct
{
    // Initialize the Snake
    void (*init)(int initial_x, int initial_y);

    // Move the Snake
    void (*move)(int x, int y);

    // Grow the Snake
    void (*grow)(void);

    // check if position collides with the snake
    bool (*check_collision)(Position pos);

    // Get the snake's head position
    Position (*get_head)(void);

    // Draw the snake
    void (*draw)(void);

    // Clean up rss
    void (*cleanup)(void);

} SnakeInterface;
#endif //SNAKE_SNAKE_INTERFACE_H
