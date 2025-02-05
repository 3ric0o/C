#include <stdio.h>
#include "raylib.h"
#include "snake_list.h"
#include "snake_array.h"

void check_dataStructure(bool using_array, SnakeInterface **snake)
{
    if(using_array)
    {
        *snake = get_snake_array();
    }
    else
    {
        *snake = get_snake_list();
    }
}

int main(void)
{
    SnakeInterface *snake = NULL;
    bool using_array = true;

    check_dataStructure(using_array, &snake);
    snake->init(800,450);

    InitWindow(1600,900,"Snake Game");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(SKYBLUE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
