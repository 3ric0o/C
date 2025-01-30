#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define BALL_DAMAGE 1

#define MIN_BALL_SPEED 800.0f
#define MAX_BALL_SPEED 1400.0f
#define BASE_BALL_SPEED 900.0f

#define BLOCKS_COLUMNS 8
#define BLOCKS_ROWS 5
#define BLOCK_WIDTH 150
#define BLOCK_HEIGHT 30
#define BLOCK_PADDING 2

#define MAX_POWERUPS 5
#define POWERUP_DROP_CHANCE 0.2f
#define POWERUP_SIZE 30
#define POWERUP_FALL_SPEED 150.0f
#define POWERUP_DURATION 15.0f

#define WIDTH           GetScreenWidth()
#define HEIGHT          GetScreenHeight()
#define STARS           200
#define SCROLL_SPEED    0.01f

#define MAX_PARTICLES 100
#define PARTICLE_LIFETIME 1.0f  // Lifetime in seconds

#define MAX_LASERS 10
#define LASER_SPEED 800.0f
#define LASER_COOLDOWN 0.2f  // Time between shots in seconds

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float lifetime;
    bool active;
} Particle;

typedef struct {
    Particle particles[MAX_PARTICLES];
    int count;
} ParticleSystem;

typedef struct {
    float x;    //The stars coordinates
    float y;    //on the screen
    float z;    //The stars depth or distance from camera
}Star;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
    Color color;
    float max_speed;
    int health;
} player_t;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;
} ball_t;

typedef enum {
    BLOCK_NORMAL,
    BLOCK_EXPLOSIVE,
    BLOCK_GHOST,
    BLOCK_MOVING,
    BLOCK_INDESTRUCTIBLE
} BlockType;

typedef struct {
    Vector2 position;
    Vector2 size;
    Color color;
    bool active;
    bool visible;
    int hits_required;
    int current_hits;
    BlockType type;
    float ghost_timer;
    float move_direction;
} block_t;

const Color BLOCK_COLORS[] = {GREEN, ORANGE, RED, PURPLE, BLUE};
const int BLOCK_HITS[] = {1, 2, 3, 4, 5};

typedef enum {
    PLAYING,
    GAME_OVER
} GameState;

typedef struct {
    float angle;
    bool visible;
} LaunchArrow;

typedef enum {
    POWERUP_ENLARGE,
    POWERUP_SPEED,
    POWERUP_LASER,
    POWERUP_STICKY
} PowerUpType;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    PowerUpType type;
    bool active;
    float radius;
} PowerUp;

typedef struct {
    bool is_stuck;           // Is the ball currently stuck to the paddle?
    Vector2 stick_offset;    // Relative position of the ball to the paddle when stuck
} StickyState;

StickyState sticky_state = { false, (Vector2){0, 0} };

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
    Color color;
    float width;
    float height;
} Laser;

Laser lasers[MAX_LASERS];
float laser_cooldown = 0.0f;

PowerUp powerups[MAX_POWERUPS];
float powerup_timer = 0.0f;
bool has_sticky_powerup = false;
bool has_laser_powerup = false;

void InitializeLasers() {
    for (int i = 0; i < MAX_LASERS; i++) {
        lasers[i].active = false;
    }
}

void ShootLaser(Vector2 position) {
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!lasers[i].active) {
            lasers[i].position = position;
            lasers[i].velocity = (Vector2){0, -LASER_SPEED};
            lasers[i].active = true;
            lasers[i].color = RED;
            lasers[i].width = 4;
            lasers[i].height = 15;
            break;
        }
    }
}

void UpdateLasers(Laser* lasers, block_t blocks[BLOCKS_ROWS][BLOCKS_COLUMNS], float delta_time) {
    // Update laser cooldown
    if (laser_cooldown > 0) {
        laser_cooldown -= delta_time;
    }

    // Update each laser
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!lasers[i].active) continue;

        // Move laser
        lasers[i].position.y += lasers[i].velocity.y * delta_time;

        // Check if laser is off screen
        if (lasers[i].position.y < 0) {
            lasers[i].active = false;
            continue;
        }

        // Check collision with blocks
        bool collision = false;
        for (int row = 0; row < BLOCKS_ROWS && !collision; row++) {
            for (int col = 0; col < BLOCKS_COLUMNS && !collision; col++) {
                if (!blocks[row][col].active ||
                    (blocks[row][col].type == BLOCK_GHOST && !blocks[row][col].visible)) continue;

                Rectangle block = {
                        blocks[row][col].position.x,
                        blocks[row][col].position.y,
                        blocks[row][col].size.x,
                        blocks[row][col].size.y
                };

                Rectangle laser = {
                        lasers[i].position.x - lasers[i].width/2,
                        lasers[i].position.y - lasers[i].height/2,
                        lasers[i].width,
                        lasers[i].height
                };

                if (CheckCollisionRecs(laser, block)) {
                    collision = true;
                    lasers[i].active = false;

                    // Handle block hit
                    if (blocks[row][col].type != BLOCK_INDESTRUCTIBLE) {
                        blocks[row][col].current_hits += BALL_DAMAGE;
                        if (blocks[row][col].current_hits >= blocks[row][col].hits_required) {
                            blocks[row][col].active = false;
                        }
                    }
                }
            }
        }
    }
}

void DrawLasers(Laser* lasers) {
    for (int i = 0; i < MAX_LASERS; i++) {
        if (lasers[i].active) {
            DrawRectangle(
                    lasers[i].position.x - lasers[i].width/2,
                    lasers[i].position.y - lasers[i].height/2,
                    lasers[i].width,
                    lasers[i].height,
                    lasers[i].color
            );
        }
    }
}

void DrawArrow(Vector2 position, float angle, Color color) {
    float arrowLength = 60.0f;
    Vector2 end = {
            position.x + cosf(angle*DEG2RAD) * arrowLength,
            position.y + sinf(angle*DEG2RAD) * arrowLength
    };

    DrawLineEx(position, end, 3.0f, color);

    Vector2 arrow1 = {
            end.x + cosf((angle+150)*DEG2RAD) * 15,
            end.y + sinf((angle+150)*DEG2RAD) * 15
    };
    Vector2 arrow2 = {
            end.x + cosf((angle-150)*DEG2RAD) * 15,
            end.y + sinf((angle-150)*DEG2RAD) * 15
    };
    DrawLineEx(end, arrow1, 3.0f, color);
    DrawLineEx(end, arrow2, 3.0f, color);
}

// Initialize a power-up
void InitPowerUp(PowerUp* powerup, Vector2 position) {
    powerup->position = position;
    powerup->velocity = (Vector2){0, POWERUP_FALL_SPEED};
    powerup->active = true;
    powerup->type = GetRandomValue(0, 3);  // Random power-up type
    powerup->radius = POWERUP_SIZE / 2;
    };

// Apply power-up effects
void ApplyPowerUp(PowerUp* powerup, player_t* player) {
    switch (powerup->type) {
        case POWERUP_ENLARGE:
            player->size.x = fmin(player->size.x * 1.5f, GetScreenWidth() / 3);
            break;
        case POWERUP_SPEED:
            player->max_speed = fmin(player->max_speed * 1.5f, 1200.0f);
            break;
        case POWERUP_LASER:
            has_laser_powerup = true;
            break;
        case POWERUP_STICKY:
            has_sticky_powerup = true;
            break;
    }
    powerup_timer = POWERUP_DURATION;
}

// Update power-ups
void UpdatePowerUps(PowerUp* powerups, player_t* player, float delta_time) {

    static const float ORIGINAL_SIZE_X = 150.0f;
    static const float ORIGINAL_SPEED = 800.0f;
    // Update power-up timer
    if (powerup_timer > 0)
    {
        powerup_timer -= delta_time;
        if (powerup_timer <= 0) {
            // Reset power-up effects using the original values
            player->size.x = ORIGINAL_SIZE_X;
            player->max_speed = ORIGINAL_SPEED;
            has_laser_powerup = false;
            has_sticky_powerup = false;
            sticky_state.is_stuck = false;
        }
    }
    // Update active power-ups
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups[i].active) continue;

        powerups[i].position.y += powerups[i].velocity.y * delta_time;

        // Check collision with paddle using circle-rectangle collision
        if (CheckCollisionCircleRec(powerups[i].position, powerups[i].radius,
                                    (Rectangle){player->position.x, player->position.y,
                                                player->size.x, player->size.y})) {
            ApplyPowerUp(&powerups[i], player);
            powerups[i].active = false;
        }

        // Deactivate if falls below screen
        if (powerups[i].position.y > GetScreenHeight()) {
            powerups[i].active = false;
        }
    }
}

// Draw power-ups
void DrawPowerUps(PowerUp* powerups) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups[i].active) continue;

        Color powerup_color;
        const char* powerup_text = "";

        switch (powerups[i].type) {
            case POWERUP_ENLARGE:
                powerup_color = GREEN;
                powerup_text = "E";
                break;
            case POWERUP_SPEED:
                powerup_color = YELLOW;
                powerup_text = "S";
                break;
            case POWERUP_LASER:
                powerup_color = RED;
                powerup_text = "L";
                break;
            case POWERUP_STICKY:
                powerup_color = PURPLE;
                powerup_text = "ST";
                break;
        }
        DrawCircleV(powerups[i].position, powerups[i].radius, powerup_color);
        int textWidth = MeasureText(powerup_text, 15);
        DrawText(powerup_text,
                 powerups[i].position.x - textWidth/2,
                 powerups[i].position.y - 7,
                 15, WHITE);
    }
}

// Update blocks with special types
void UpdateBlocks(block_t blocks[BLOCKS_ROWS][BLOCKS_COLUMNS], float delta_time) {
    for (int i = 0; i < BLOCKS_ROWS; i++) {
        for (int j = 0; j < BLOCKS_COLUMNS; j++) {
            if (!blocks[i][j].active) continue;

            switch (blocks[i][j].type) {
                case BLOCK_GHOST:
                    blocks[i][j].ghost_timer += delta_time;
                    if (blocks[i][j].ghost_timer >= 2.0f) {
                        blocks[i][j].ghost_timer = 0.0f;
                        blocks[i][j].visible = !blocks[i][j].visible;
                    }
                    break;

                case BLOCK_MOVING:
                    blocks[i][j].position.x += blocks[i][j].move_direction * 100.0f * delta_time;
                    if (blocks[i][j].position.x <= 0 ||
                        blocks[i][j].position.x >= GetScreenWidth() - blocks[i][j].size.x) {
                        blocks[i][j].move_direction *= -1;
                    }
                    break;

                case BLOCK_EXPLOSIVE:
                    // TODO Handle explosion when hit
                    break;

                default:
                    break;
            }
        }
    }
}
void InitializeLevel(block_t blocks[BLOCKS_ROWS][BLOCKS_COLUMNS], int level, float offset_x, float offset_y) {
    // Clear existing blocks
    for (int i = 0; i < BLOCKS_ROWS; i++) {
        for (int j = 0; j < BLOCKS_COLUMNS; j++) {
            blocks[i][j].active = false;
        }
    }

    switch (level) {
        case 1:
            // First, place regular blocks
            for (int i = 0; i < BLOCKS_ROWS - 1; i++) {  // Leave last row for ghost blocks
                for (int j = 0; j < BLOCKS_COLUMNS; j++) {
                    int color_index = i % 5;
                    blocks[i][j] = (block_t){
                            .position = (Vector2){j * (BLOCK_WIDTH + BLOCK_PADDING) + offset_x,
                                                  i * (BLOCK_HEIGHT + BLOCK_PADDING) + offset_y},
                            .size = (Vector2){BLOCK_WIDTH, BLOCK_HEIGHT},
                            .color = BLOCK_COLORS[color_index],
                            .active = true,
                            .hits_required = BLOCK_HITS[color_index],
                            .current_hits = 0,
                            .type = BLOCK_NORMAL,
                            .ghost_timer = 0.0f,
                            .move_direction = 1.0f
                    };

                    // Add explosive blocks in the middle
                    if (i == 2 && (j == 3 || j == 4)) {
                        blocks[i][j].type = BLOCK_EXPLOSIVE;
                    }
                    if (i == 0 && (j == 0 || j == 7)) {
                        blocks[i][j].type = BLOCK_INDESTRUCTIBLE;
                    }
                }
            }
            // Add ghost blocks in the last row
            for (int j = 2; j < 6; j++) {  // Place ghost blocks in middle positions
                blocks[BLOCKS_ROWS-1][j] = (block_t){
                        .position = (Vector2){j * (BLOCK_WIDTH + BLOCK_PADDING) + offset_x,
                                              (BLOCKS_ROWS-1) * (BLOCK_HEIGHT + BLOCK_PADDING) + offset_y},
                        .size = (Vector2){BLOCK_WIDTH, BLOCK_HEIGHT},
                        .color = SKYBLUE,
                        .active = true,
                        .hits_required = 1,
                        .current_hits = 0,
                        .type = BLOCK_GHOST,
                        .visible = true,
                        .ghost_timer = 0.0f,
                        .move_direction = 1.0f
                };
            }
            break;

        case 2:
            // Place regular and moving blocks first
            for (int i = 0; i < BLOCKS_ROWS - 1; i++) {
                for (int j = 0; j < BLOCKS_COLUMNS; j++) {
                    int color_index = (i + j) % 5;
                    blocks[i][j] = (block_t){
                            .position = (Vector2){j * (BLOCK_WIDTH + BLOCK_PADDING) + offset_x,
                                                  i * (BLOCK_HEIGHT + BLOCK_PADDING) + offset_y},
                            .size = (Vector2){BLOCK_WIDTH, BLOCK_HEIGHT},
                            .color = BLOCK_COLORS[color_index],
                            .active = true,
                            .hits_required = BLOCK_HITS[color_index] + 1,
                            .current_hits = 0,
                            .type = BLOCK_NORMAL,
                            .ghost_timer = 0.0f,
                            .move_direction = 1.0f
                    };

                    if (i == 2 && j == 4) {
                        blocks[i][j].type = BLOCK_EXPLOSIVE;
                    }
                    if (i == 3 && (j % 2 == 1)) {
                        blocks[i][j].type = BLOCK_MOVING;
                    }
                }
            }
            // Add ghost blocks in bottom row with alternating pattern
            for (int j = 0; j < BLOCKS_COLUMNS; j += 2) {
                blocks[BLOCKS_ROWS-1][j] = (block_t){
                        .position = (Vector2){j * (BLOCK_WIDTH + BLOCK_PADDING) + offset_x,
                                              (BLOCKS_ROWS-1) * (BLOCK_HEIGHT + BLOCK_PADDING) + offset_y},
                        .size = (Vector2){BLOCK_WIDTH, BLOCK_HEIGHT},
                        .color = SKYBLUE,
                        .active = true,
                        .hits_required = 1,
                        .current_hits = 0,
                        .type = BLOCK_GHOST,
                        .visible = true,
                        .ghost_timer = 0.0f,
                        .move_direction = 1.0f
                };
            }
            break;

        case 3:
            // Place regular blocks and other special blocks
            for (int i = 0; i < BLOCKS_ROWS - 1; i++) {
                for (int j = 0; j < BLOCKS_COLUMNS; j++) {
                    int color_index = 4 - (i % 5);
                    blocks[i][j] = (block_t){
                            .position = (Vector2){j * (BLOCK_WIDTH + BLOCK_PADDING) + offset_x,
                                                  i * (BLOCK_HEIGHT + BLOCK_PADDING) + offset_y},
                            .size = (Vector2){BLOCK_WIDTH, BLOCK_HEIGHT},
                            .color = BLOCK_COLORS[color_index],
                            .active = true,
                            .hits_required = BLOCK_HITS[color_index] + 2,
                            .current_hits = 0,
                            .type = BLOCK_NORMAL,
                            .ghost_timer = 0.0f,
                            .move_direction = 1.0f
                    };

                    if ((i + j) % 2 == 0 && i < 2) {
                        blocks[i][j].type = BLOCK_INDESTRUCTIBLE;
                    }
                    if (i == 2 && (j == 2 || j == 5)) {
                        blocks[i][j].type = BLOCK_EXPLOSIVE;
                    }
                    if (i == 3) {
                        blocks[i][j].type = BLOCK_MOVING;
                    }
                }
            }
            // Add ghost blocks in a zigzag pattern in bottom row
            for (int j = 0; j < BLOCKS_COLUMNS; j++) {
                if (j % 3 == 0) {
                    blocks[BLOCKS_ROWS-1][j] = (block_t){
                            .position = (Vector2){j * (BLOCK_WIDTH + BLOCK_PADDING) + offset_x,
                                                  (BLOCKS_ROWS-1) * (BLOCK_HEIGHT + BLOCK_PADDING) + offset_y},
                            .size = (Vector2){BLOCK_WIDTH, BLOCK_HEIGHT},
                            .color = SKYBLUE,
                            .active = true,
                            .hits_required = 1,
                            .current_hits = 0,
                            .type = BLOCK_GHOST,
                            .visible = true,
                            .ghost_timer = 0.0f,
                            .move_direction = 1.0f
                    };
                }
            }
            break;
    }
}
bool IsLevelComplete(block_t blocks[BLOCKS_ROWS][BLOCKS_COLUMNS]) {
    for (int i = 0; i < BLOCKS_ROWS; i++) {
        for (int j = 0; j < BLOCKS_COLUMNS; j++) {
            if (blocks[i][j].active && blocks[i][j].type != BLOCK_INDESTRUCTIBLE) {
                return false;
            }
        }
    }
    return true;
}
void NormalizeBallSpeed(ball_t* ball) {
    float speed = Vector2Length(ball->velocity);
    if (speed < MIN_BALL_SPEED || speed > MAX_BALL_SPEED) {
        speed = Clamp(speed, MIN_BALL_SPEED, MAX_BALL_SPEED);
        ball->velocity = Vector2Scale(Vector2Normalize(ball->velocity), speed);
    }
}
void InitParticleSystem(ParticleSystem* system) {
    system->count = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        system->particles[i].active = false;
    }
}
void CreateExplosion(ParticleSystem* system, Vector2 position, Color color) {
    int particlesToCreate = 50;

    for (int i = 0; i < particlesToCreate && system->count < MAX_PARTICLES; i++) {
        Particle* p = &system->particles[system->count];

        p->position = position;

        // Random angle for particle direction
        float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
        float speed = (float)GetRandomValue(100, 300);
        p->velocity.x = cosf(angle) * speed;
        p->velocity.y = sinf(angle) * speed;

        // Random color variation
        p->color = color;
        p->color.r = (unsigned char)Clamp(color.r + GetRandomValue(-40, 40), 0, 255);
        p->color.g = (unsigned char)Clamp(color.g + GetRandomValue(-40, 40), 0, 255);
        p->color.b = (unsigned char)Clamp(color.b + GetRandomValue(-40, 40), 0, 255);

        p->lifetime = PARTICLE_LIFETIME;
        p->active = true;

        system->count++;
    }
}
void UpdateParticleSystem(ParticleSystem* system, float delta_time) {
    for (int i = 0; i < system->count; i++) {
        if (!system->particles[i].active) continue;

        // Update particle position
        system->particles[i].position.x += system->particles[i].velocity.x * delta_time;
        system->particles[i].position.y += system->particles[i].velocity.y * delta_time;

        // Update lifetime and alpha
        system->particles[i].lifetime -= delta_time;
        system->particles[i].color.a = (unsigned char)(255.0f * (system->particles[i].lifetime / PARTICLE_LIFETIME));

        // Add some gravity effect
        system->particles[i].velocity.y += 400.0f * delta_time;

        // Deactivate if lifetime is over
        if (system->particles[i].lifetime <= 0) {
            system->particles[i].active = false;

            // Swap with last active particle and decrease count
            if (i != system->count - 1) {
                system->particles[i] = system->particles[system->count - 1];
                i--;
            }
            system->count--;
        }
    }
}
void DrawParticleSystem(ParticleSystem* system) {
    for (int i = 0; i < system->count; i++) {
        if (system->particles[i].active) {
            DrawCircleV(system->particles[i].position, 5, system->particles[i].color);
        }
    }
}

float randf() { return (rand() % 1000) / 1000.0f; }

int main() {
    InitWindow(1600, 1200, "Block Kuzushi");

    ParticleSystem particleSystem;
    InitParticleSystem(&particleSystem);
    InitializeLasers();
    Star stars[STARS] = {0};
    srand(time(0));
    int level = 1;
    int score = 0;
    GameState gameState = PLAYING;
    bool ball_launched = false;

    //-----RANDOMISE THE STARS POSITIONS-----//
    for (int i = 0; i < STARS; i++) {
        stars[i].x = GetRandomValue(0, WIDTH);
        stars[i].y = GetRandomValue(0, HEIGHT);
        stars[i].z = randf();
    }

    LaunchArrow arrow =
            {
            .angle = -90.0f,
            .visible = true
            };

    const float ARROW_SPEED = 200.0f;

    // Initialize player
    player_t player =
            {
            .position = (Vector2){800, 1025},
            .velocity = (Vector2){0, 0},
            .size = (Vector2){150, 30},
            .color = RED,
            .max_speed = 800,
            .health = 3
            };

    // Initialize ball
    ball_t ball =
            {
            .position = (Vector2){800, 1000},
            .velocity = (Vector2){0, -0},
            .radius = 10,
            .color = WHITE
            };

    // Initialize blocks with different colors and hit points
    block_t blocks[BLOCKS_ROWS][BLOCKS_COLUMNS];
    float offset_x = (GetScreenWidth() - (BLOCKS_COLUMNS * (BLOCK_WIDTH + BLOCK_PADDING))) / 2;
    float offset_y = 50;

    // Initialize first level
    InitializeLevel(blocks, level, offset_x, offset_y);
    for (int i = 0; i < MAX_POWERUPS; i++) {
        powerups[i].active = false;
    }

    char score_text[32];

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        //-----SCROLL THE STARS-----//
        for (int i = 0; i < STARS; i++) {
            stars[i].x -= SCROLL_SPEED * (stars[i].z / 1);

            if (stars[i].x <= 0) {  // Check if the star has gone offscreen
                stars[i].x += WIDTH;
                stars[i].y = GetRandomValue(0, HEIGHT);
            }
        }

        //-----DRAW THE STARS-----//
        BeginDrawing();
        ClearBackground((Color){0, 0, 0, 255});
        for (int i = 0; i < STARS; i++) {
            float x = stars[i].x;
            float y = stars[i].y;

            DrawPixel(x, y, WHITE);
        }

        if (gameState == PLAYING) {
            player.velocity = (Vector2) {0.0f, 0.0f};

            if (IsKeyDown(KEY_A)) {
                player.velocity.x = -1;
            } else if (IsKeyDown(KEY_D)) {
                player.velocity.x = 1;
            }

            // Update player position
            if (Vector2Length(player.velocity) > 0) {
                player.velocity = Vector2Scale(Vector2Normalize(player.velocity), player.max_speed * delta_time);
            }
            player.position = Vector2Add(player.position, player.velocity);
            // Keep player within screen bounds
            player.position.x = Clamp(player.position.x, 0, GetScreenWidth() - player.size.x);


            if (has_laser_powerup && laser_cooldown <= 0) {
                Vector2 laserPos = {
                        player.position.x + player.size.x/2,
                        player.position.y
                };
                ShootLaser(laserPos);
                laser_cooldown = LASER_COOLDOWN;
            }

            if (!ball_launched) {
                // Keep ball on paddle
                ball.position = (Vector2){
                        player.position.x + player.size.x / 2,
                        player.position.y - ball.radius
                };

                // Add arrow movement
                static float direction = 1.0f;  // 1 for right, -1 for left
                arrow.angle += (ARROW_SPEED * direction * delta_time);

                // Reverse direction when reaching limits
                if (arrow.angle >= 0) {
                    direction = -1.0f;
                    arrow.angle = 0;
                }
                else if (arrow.angle <= -180) {
                    direction = 1.0f;
                    arrow.angle = -180;
                }

                // Launch ball when space is pressed
                if (IsKeyPressed(KEY_SPACE)) {
                    ball_launched = true;
                    arrow.visible = false;

                    // Convert angle to radians and set ball velocity
                    float radians = arrow.angle * DEG2RAD;
                    ball.velocity.x = cosf(radians) * BASE_BALL_SPEED;
                    ball.velocity.y = sinf(radians) * BASE_BALL_SPEED;
                    NormalizeBallSpeed(&ball);
                }
            }
            else {  // ball is launched
                if (sticky_state.is_stuck) {
                    // Update ball position relative to paddle
                    ball.position.x = player.position.x + player.size.x/2 + sticky_state.stick_offset.x;
                    ball.position.y = player.position.y + sticky_state.stick_offset.y;

                    // Add arrow movement when ball is stuck
                    arrow.visible = true;
                    static float direction = 1.0f;  // 1 for right, -1 for left
                    arrow.angle += (ARROW_SPEED * direction * delta_time);

                    // Reverse direction when reaching limits
                    if (arrow.angle >= 0) {
                        direction = -1.0f;
                        arrow.angle = 0;
                    }
                    else if (arrow.angle <= -180) {
                        direction = 1.0f;
                        arrow.angle = -180;
                    }

                    // Check for launch input
                    if (IsKeyPressed(KEY_SPACE)) {
                        sticky_state.is_stuck = false;
                        arrow.visible = false;
                        // Calculate launch angle using the arrow angle instead of paddle position
                        float radians = arrow.angle * DEG2RAD;
                        ball.velocity.x = cosf(radians) * BASE_BALL_SPEED;
                        ball.velocity.y = sinf(radians) * BASE_BALL_SPEED;
                        NormalizeBallSpeed(&ball);
                    }
                }
                else {  // ball is launched and not stuck
                    // 1. First update ball position
                    Vector2 next_position = Vector2Add(ball.position, Vector2Scale(ball.velocity, delta_time));
                    ball.position = next_position;

                    // 2. Handle wall collisions first
                    if (ball.position.x >= GetScreenWidth() - ball.radius) {
                        ball.position.x = GetScreenWidth() - ball.radius;
                        ball.velocity.x = -fabsf(ball.velocity.x);
                    }
                    else if (ball.position.x <= ball.radius) {
                        ball.position.x = ball.radius;
                        ball.velocity.x = fabsf(ball.velocity.x);
                    }

                    if (ball.position.y <= ball.radius) {
                        ball.position.y = ball.radius;
                        ball.velocity.y = fabsf(ball.velocity.y);
                    }

                    // 3. Handle paddle collision
                    if (CheckCollisionCircleRec(ball.position, ball.radius,
                                                (Rectangle){player.position.x, player.position.y, player.size.x, player.size.y})) {
                        if (has_sticky_powerup && !sticky_state.is_stuck) {
                            // Ball becomes stuck to the paddle
                            sticky_state.is_stuck = true;
                            // Calculate and store the offset from the paddle's center
                            sticky_state.stick_offset.x = ball.position.x - (player.position.x + player.size.x/2);
                            sticky_state.stick_offset.y = -(ball.radius + player.size.y/2);
                            ball.velocity = (Vector2){0, 0}; // Stop the ball
                        }
                        else {
                            // Normal paddle collision code
                            float hitPosition = (ball.position.x - player.position.x) / player.size.x;
                            float angle = (hitPosition - 0.5f) * 120.0f;
                            float radians = angle * DEG2RAD;
                            float speed = Vector2Length(ball.velocity);
                            speed = Clamp(speed, MIN_BALL_SPEED, MAX_BALL_SPEED);
                            ball.velocity.x = sinf(radians) * speed;
                            ball.velocity.y = -fabsf(cosf(radians) * speed);
                            ball.position.y = player.position.y - ball.radius - 1;
                        }
                    }
                }
                // 4. Handle block collisions
                bool collision_handled = false;
                for (int i = 0; i < BLOCKS_ROWS && !collision_handled; i++) {
                    for (int j = 0; j < BLOCKS_COLUMNS && !collision_handled; j++) {
                        if (!blocks[i][j].active || (blocks[i][j].type == BLOCK_GHOST && !blocks[i][j].visible)) continue;

                        Rectangle block = {
                                blocks[i][j].position.x,
                                blocks[i][j].position.y,
                                blocks[i][j].size.x,
                                blocks[i][j].size.y
                        };

                        if (CheckCollisionCircleRec(ball.position, ball.radius, block)) {
                            collision_handled = true;

                            // Handle block hit first
                            if (blocks[i][j].type == BLOCK_EXPLOSIVE) {
                                // Create explosion effect
                                CreateExplosion(&particleSystem,
                                                (Vector2){blocks[i][j].position.x + blocks[i][j].size.x/2,
                                                          blocks[i][j].position.y + blocks[i][j].size.y/2},
                                                RED);
                                // Handle explosive blocks
                                for (int di = -1; di <= 1; di++) {
                                    for (int dj = -1; dj <= 1; dj++) {
                                        int ni = i + di;
                                        int nj = j + dj;
                                        if (ni >= 0 && ni < BLOCKS_ROWS && nj >= 0 && nj < BLOCKS_COLUMNS) {
                                            blocks[ni][nj].active = false;
                                        }
                                    }
                                }
                                blocks[i][j].active = false;
                            } else if (blocks[i][j].type != BLOCK_INDESTRUCTIBLE) {
                                blocks[i][j].current_hits += BALL_DAMAGE;
                                if (blocks[i][j].current_hits >= blocks[i][j].hits_required) {
                                    blocks[i][j].active = false;
                                    score += blocks[i][j].hits_required * 100;

                                    // Handle power-up spawn
                                    if (GetRandomValue(0, 100) < POWERUP_DROP_CHANCE * 100) {
                                        for (int p = 0; p < MAX_POWERUPS; p++) {
                                            if (!powerups[p].active) {
                                                InitPowerUp(&powerups[p], blocks[i][j].position);
                                                break;
                                            }
                                        }
                                    }
                                }
                            }

                            // Get previous position
                            Vector2 prev_pos = Vector2Subtract(ball.position, Vector2Scale(ball.velocity, delta_time));

                            // Find closest point on block to previous position
                            Vector2 closest = {
                                    Clamp(prev_pos.x, block.x, block.x + block.width),
                                    Clamp(prev_pos.y, block.y, block.y + block.height)
                            };

                            // Calculate normal from closest point
                            Vector2 normal = Vector2Normalize(Vector2Subtract(prev_pos, closest));

                            if (Vector2Length(normal) < 0.1f) {
                                float dx = ball.position.x - (block.x + block.width / 2);
                                float dy = ball.position.y - (block.y + block.height / 2);
                                if (fabsf(dx) > fabsf(dy)) {
                                    normal = (Vector2) {(dx > 0) ? 1.0f : -1.0f, 0};
                                } else {
                                    normal = (Vector2) {0, (dy > 0) ? 1.0f : -1.0f};
                                }
                            }

                            // Reflect velocity
                            float dotProduct = Vector2DotProduct(ball.velocity, normal);
                            ball.velocity = Vector2Subtract(ball.velocity,
                                                            Vector2Scale(normal, 2 * dotProduct));

                            // Move ball out of collision
                            float overlap = ball.radius - Vector2Distance(ball.position, closest);
                            if (overlap > 0) {
                                ball.position = Vector2Add(ball.position,
                                                           Vector2Scale(normal, overlap + 1.0f));
                            }

                            // Maintain minimum angles
                            float speed = Vector2Length(ball.velocity);
                            if (fabsf(ball.velocity.y) < speed * 0.2f) {
                                ball.velocity.y = (ball.velocity.y >= 0 ? 1 : -1) * speed * 0.2f;
                                ball.velocity = Vector2Scale(Vector2Normalize(ball.velocity), speed);
                            }
                            if (fabsf(ball.velocity.x) < speed * 0.2f) {
                                ball.velocity.x = (ball.velocity.x >= 0 ? 1 : -1) * speed * 0.2f;
                                ball.velocity = Vector2Scale(Vector2Normalize(ball.velocity), speed);
                            }
                            NormalizeBallSpeed(&ball);
                        }

                    }
                }
            }
            if (ball.position.y > GetScreenHeight()) {
                player.health--;
                if (player.health <= 0) {
                    gameState = GAME_OVER;
                } else {
                    ball_launched = false;
                    arrow.visible = true;
                    ball.position = (Vector2){player.position.x + player.size.x / 2,
                                              player.position.y - ball.radius};
                    ball.velocity = (Vector2){0, 0};
                }
            }
            if (IsLevelComplete(blocks))
            {
                level++;
                if (level <= 3)
                {
                    // Initialize next level
                    InitializeLevel(blocks, level, offset_x, offset_y);
                    // Reset ball
                    ball_launched = false;
                    ball.position = (Vector2) {player.position.x + player.size.x / 2,
                                               player.position.y - ball.radius};
                    ball.velocity = (Vector2) {0, 0};
                    arrow.visible = true;
                } else
                {
                    // Player won the game
                    gameState = GAME_OVER;
                    score += 1000 * player.health; // Bonus points for remaining health
                }
            }

            UpdatePowerUps(powerups, &player, delta_time);
            UpdateBlocks(blocks, delta_time);
            UpdateLasers(lasers, blocks, delta_time);
        }
        ClearBackground(BLACK);

        if (gameState == PLAYING) {
            DrawRectangleV(player.position, player.size, player.color);
            DrawCircleV(ball.position, ball.radius, ball.color);
            DrawLasers(lasers);

            // Draw blocks
            for (int i = 0; i < BLOCKS_ROWS; i++) {
                for (int j = 0; j < BLOCKS_COLUMNS; j++) {
                    if (blocks[i][j].active && (blocks[i][j].type != BLOCK_GHOST || blocks[i][j].visible)) {
                        Color blockColor;
                        char blockText[3];  // Buffer for block text

                        // Set color and text based on block type
                        switch (blocks[i][j].type) {
                            case BLOCK_NORMAL:
                                blockColor = GRAY;  // All normal blocks are gray
                                sprintf(blockText, "%d", blocks[i][j].hits_required - blocks[i][j].current_hits);
                                break;
                            case BLOCK_EXPLOSIVE:
                                blockColor = RED;
                                strcpy(blockText, "X");
                                break;
                            case BLOCK_GHOST:
                                blockColor = SKYBLUE;
                                strcpy(blockText, "G");
                                blockColor.a = 128; // Semi-transparent
                                break;
                            case BLOCK_MOVING:
                                blockColor = YELLOW;
                                strcpy(blockText, "M");
                                break;
                            case BLOCK_INDESTRUCTIBLE:
                                blockColor = WHITE;
                                strcpy(blockText, "I");
                                break;
                            default:
                                blockColor = GRAY;
                                blockText[0] = '\0';
                                break;
                        }

                        // Draw the block
                        DrawRectangleV(blocks[i][j].position, blocks[i][j].size, blockColor);

                        // Center the text in the block
                        int textWidth = MeasureText(blockText, 20);
                        Vector2 textPos = {
                                blocks[i][j].position.x + (blocks[i][j].size.x - textWidth) / 2,
                                blocks[i][j].position.y + (blocks[i][j].size.y - 20) / 2
                        };

                        // Draw the text
                        DrawText(blockText, textPos.x, textPos.y, 20,
                                 blocks[i][j].type == BLOCK_NORMAL ? BLACK : WHITE);
                    }
                }
            }

            if ((!ball_launched || sticky_state.is_stuck) && arrow.visible) {
                Vector2 arrowStart = ball.position;

                DrawArrow(arrowStart, arrow.angle, RED);

                const char *launchText = "Press SPACE to launch";
                int textWidth = MeasureText(launchText, 50);
                DrawText(launchText,
                         GetScreenWidth() / 2 - textWidth / 2,
                         GetScreenHeight() / 2,
                         50, WHITE);
            }

            // Draw score and Health and Level
            sprintf(score_text, "Health: %d", player.health);
            DrawText(score_text, 1460, 10, 30, WHITE);

            sprintf(score_text, "Score: %d", score);
            DrawText(score_text, 150, 10, 30, WHITE);

            sprintf(score_text, "Level: %d", level);
            DrawText(score_text, 10, 10, 30, WHITE);

            DrawPowerUps(powerups);

            // Draw power-up timer if active
            if (powerup_timer > 0) {
                char timer_text[32];
                sprintf(timer_text, "Power-up: %.1f", powerup_timer);
                DrawText(timer_text, 10, 90, 20, YELLOW);
            }
            // Draw active power-up indicators
            if (has_sticky_powerup) {
                DrawText("STICKY", 10, 120, 20, PURPLE);
                if (sticky_state.is_stuck) {
                    const char* launchText = "Press SPACE to Launch";
                    int textWidth = MeasureText(launchText, 20);
                    DrawText(launchText,
                             ball.position.x - textWidth/2,
                             ball.position.y - 30,
                             20,
                             PURPLE);
                }
            }
            if (has_laser_powerup) {
                DrawText("LASER", 10, 150, 20, RED);
            }
        }
        else // GAME_OVER state
        {
            // Draw Game Over screen
            const char *gameOverText = level > 3 ? "YOU WIN!" : "GAME OVER";
            char finalScoreText[32];
            sprintf(finalScoreText, "Final Score: %d", score);
            const char *restartText = "Press R to Restart";

            // Center the text
            int gameOverWidth = MeasureText(gameOverText, 60);
            int scoreWidth = MeasureText(finalScoreText, 30);
            int restartWidth = MeasureText(restartText, 20);

            Color titleColor = level > 3 ? GREEN : RED;

            DrawText(gameOverText,
                     GetScreenWidth() / 2 - gameOverWidth / 2,
                     GetScreenHeight() / 2 - 60,
                     60, titleColor);
            DrawText(finalScoreText,
                     GetScreenWidth() / 2 - scoreWidth / 2,
                     GetScreenHeight() / 2,
                     30, WHITE);
            DrawText(restartText,
                     GetScreenWidth() / 2 - restartWidth / 2,
                     GetScreenHeight() / 2 + 60,
                     20, WHITE);

            // Check for restart
            if (IsKeyPressed(KEY_R)) {
                gameState = PLAYING;
                level = 1;  // Reset to level 1
                score = 0;
                player.health = 3;
                ball.position = (Vector2) {800, 1000};
                ball.velocity = (Vector2) {500, -500};
                ball_launched = false;
                arrow.visible = true;

                // Reset all power-ups
                for (int i = 0; i < MAX_POWERUPS; i++) {
                    powerups[i].active = false;
                }
                powerup_timer = 0.0f;
                has_sticky_powerup = false;
                has_laser_powerup = false;

                // Initialize level 1
                InitializeLevel(blocks, level, offset_x, offset_y);
                InitializeLasers();
                laser_cooldown = 0.0f;
            }
        }
        UpdateParticleSystem(&particleSystem, delta_time);
        DrawParticleSystem(&particleSystem);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
#pragma clang diagnostic pop