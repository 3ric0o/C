#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#include "raylib.h"
#include "raymath.h"


typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
    Color color;
    float max_speed; // pixels / second
} player_t;


int main() {

    player_t player =
            {
                    .position = (Vector2){800, 1025},
                    .size = 64,32,
                    .color = RED,
                    .max_speed = 300
            };

    InitWindow(1600, 1200, "Shooter");

    while (!WindowShouldClose())
    {
        float delta_time = GetFrameTime();

        player.velocity = (Vector2){0.0f, 0.0f};

        if (IsKeyDown(KEY_A))
        {
            player.velocity.x = -1;
        }
        else if (IsKeyDown(KEY_D))
        {
            player.velocity.x = 1;
        }

        player.velocity = Vector2Scale(Vector2Normalize(player.velocity),player.max_speed * delta_time);

        player.position = Vector2Add(player.position, player.velocity);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangleV(player.position, player.size, player.color);

        EndDrawing();
    }
    return 0;
}
#pragma clang diagnostic pop