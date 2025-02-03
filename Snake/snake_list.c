#include "snake_interface.h"
#include <stdlib.h>
#include <raylib.h>

typedef struct SnakeNode
{
    Position pos;
    struct SnakeNode *next;

} SnakeNode;

typedef struct
{
    SnakeNode *head;
    int length;

} SnakeList;

static SnakeList snake_list;
static SnakeInterface snake_interface;

// Implementations of the SnakeInterface functions
static void list_init(int initial_x, int initial_y)
{
    snake_list.head = malloc(sizeof(SnakeNode));
    snake_list.head->pos.x = initial_x;
    snake_list.head->pos.y = initial_y;
    snake_list.head->next = NULL;
    snake_list.length = 1;
}

static void list_move(int x, int y)
{
    SnakeNode *current = snake_list.head;
    SnakeNode *prev = NULL;
    while(current->next != NULL)
    {
        prev = current;
        current = current->next;
    }
    current->pos = snake_list.head->pos;
    current->next = snake_list.head;
    snake_list.head = current;
    prev->next = NULL;
    snake_list.head->pos.x += x;
    snake_list.head->pos.y += y;
}

static void list_grow(void)
{
    SnakeNode *new_node = malloc(sizeof(SnakeNode));
    new_node->pos = snake_list.head->pos;
    new_node->next = snake_list.head;
    snake_list.head = new_node;
    snake_list.length++;
}

static bool list_check_collision(Position pos)
{
    SnakeNode *current = snake_list.head;
    while(current != NULL)
    {
        if(current->pos.x == pos.x && current->pos.y == pos.y)
        {
            return true;
        }
        current = current->next;
    }
    return false;
}

static Position list_get_head(void)
{
    return snake_list.head->pos;
}

static void list_draw(void)
{
    SnakeNode *current = snake_list.head;
    while(current != NULL)
    {
        DrawRectangle(current->pos.x, current->pos.y, 10, 10, GREEN);
        current = current->next;
    }
}

static void list_cleanup(void)
{
    SnakeNode *current = snake_list.head;
    while(current != NULL)
    {
        SnakeNode *next = current->next;
        free(current);
        current = next;
    }
    snake_list.head = NULL;
}

SnakeInterface* get_snake_list(void)
{
    snake_interface.init = list_init;
    snake_interface.move = list_move;
    snake_interface.grow = list_grow;
    snake_interface.check_collision = list_check_collision;
    snake_interface.get_head = list_get_head;
    snake_interface.draw = list_draw;
    snake_interface.cleanup = list_cleanup;

    return &snake_interface;
}