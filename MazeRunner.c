#include "raylib.h"
#include <raymath.h>

#define WINDOWWIDTH 800
#define WINDOWHEIGHT 600

#define PLAYER_MAX_ARROWS 500
#define MAX_MELEES 500
/*
typedef struct Player {
    int position_x;
    int position_y;
    float velocity_x;
    float velocity_y;
    int radius;
} Player; */

typedef struct Bullet {
    Vector2 position;
    Vector2 speed;
    bool active;
    Color color;
} arrow;

typedef struct Melee {
    Vector2 position;
    Vector2 direction;
    Vector2 size;
    bool active;
    Color color;
} melee;

void shootArrow(arrow arrows[],Vector2 position, Vector2 direction) {
    for (int i = 0; i < PLAYER_MAX_ARROWS; i++) {
        if (!arrows[i].active) {
            arrows[i].position = position;
            arrows[i].speed = direction;
            arrows[i].active = true;
            break;
        }
    }
}

void updateArrow(arrow arrows[]) {
    for (int i = 0; i < PLAYER_MAX_ARROWS; i++) {
        if (arrows[i].active) {
            arrows[i].position.x += arrows[i].speed.x;
            arrows[i].position.y += arrows[i].speed.y;

            if (arrows[i].position.x < 0 || arrows[i].position.x > WINDOWWIDTH || arrows[i].position.y < 0 || arrows[i].position.y > WINDOWHEIGHT) {
                arrows[i].active = false;
            }
        }
    }
}

void DrawArrow(arrow arrows[]) {
    for (int i = 0; i< PLAYER_MAX_ARROWS; i++) {
        if (arrows[i].active) {
            DrawCircleV(arrows[i].position, 5, GREEN);
        }
    }
}

void MeleeAttack(melee melees[], Vector2 position, Vector2 direction, Vector2 size/*might be able to add enemies etc here later*/) {
    for ( int i = 0; i < MAX_MELEES; i++) {
        if (!melees[i].active) {
            melees[i].position = position;  
            melees[i].active = true;
            melees[i].size = size;

            Rectangle meleeArea = {position.x - size.x / 2, position.y - size.y / 2, size.x, size.y };

            break;
        }
    }
}

void updateAttack() {
    //duration etc 
}

void DrawAttackAoe(melee melees[]) {
    for (int i = 0; i < MAX_MELEES; i++) {
        if (melees[i].active) {
            DrawRectangleV(melees[i].position,  melees[i].size, BLUE);
        }
    }
}

int main() {

    Vector2 playerPosition = { WINDOWWIDTH/2.0f, WINDOWHEIGHT/2.0f };
    float BaseSpeed = 7.5f;
    float playerSpeed = BaseSpeed;

    // dodge values
    float dodgeDistance = 10.0f;
    bool isDodging = false;
    double dodgeTime = 0.0f;
    double dodgeDuration = 0.2;

    // attack logics
    melee melees[MAX_MELEES] = {0};

    arrow arrows[PLAYER_MAX_ARROWS] = {0};

    InitWindow( WINDOWWIDTH, WINDOWHEIGHT, "Maze runner");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        Vector2 mousePosition = GetMousePosition();
        Vector2 BulletDirection = Vector2Normalize(Vector2Subtract(mousePosition, playerPosition));
        Vector2 arrowSpeed = {BulletDirection.x * 10.0f, BulletDirection.y * 10.0};

        updateArrow(arrows);

        double currentTime = GetTime();

        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            playerSpeed = BaseSpeed * 1.5f;
        } else {
            playerSpeed = BaseSpeed;
        }

        Vector2 direction = {0.0f, 0.0f};
        //movement logic for player_one
        if(IsKeyDown(KEY_W)) {
            direction.y -= playerSpeed;
        }
        if(IsKeyDown(KEY_S)) {
            direction.y += playerSpeed;
        }
        if(IsKeyDown(KEY_A)) {
            direction.x -= playerSpeed;
        }
        if(IsKeyDown(KEY_D)) {
            direction.x += playerSpeed;
        }

        if(IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
            shootArrow(arrows, playerPosition, arrowSpeed);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 meleeSize = { 50.0f, 20.0f};
            MeleeAttack(melees, playerPosition, direction, meleeSize);
        }

        // dodge logic
        if (IsKeyPressed(KEY_SPACE) && Vector2Length(direction) > 0) { // checks if space is active addiotinally if any other direction input is being used
            isDodging = true;
            dodgeTime = currentTime;
            playerPosition.x += direction.x * dodgeDistance;
            playerPosition.y += direction.y * dodgeDistance;
        }

        if (isDodging && (currentTime - dodgeTime) > dodgeDuration) {
            isDodging = false;
        }

        if (!isDodging) {
            playerPosition.x += direction.x * playerSpeed * GetFrameTime();
            playerPosition.y += direction.y * playerSpeed * GetFrameTime();
        }

        // border collision check
        if (playerPosition.x < 0 + 12) {
            playerPosition.x = 0 + 12;
        }
        if (playerPosition.x >  WINDOWWIDTH - 12) {
            playerPosition.x = WINDOWWIDTH - 12;
        }
        if (playerPosition.y < 0 + 12) {
            playerPosition.y = 0 + 12;
        }
        if (playerPosition.y > WINDOWHEIGHT - 12) {
            playerPosition.y =  WINDOWHEIGHT - 12;
        }

        // add an larger if statement restricting melee attacks when dodging or shooting arrows


        BeginDrawing();
        ClearBackground(BLACK);


        DrawCircleV(playerPosition, 12, RED);
        DrawArrow(arrows);
        DrawAttackAoe(melees);
        EndDrawing();
    }

    CloseWindow();
}