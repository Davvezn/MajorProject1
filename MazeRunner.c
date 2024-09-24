#include "raylib.h"
#include <raymath.h>

#define WINDOWWIDTH 800
#define WINDOWHEIGHT 600

typedef struct Player {
    int position_x;
    int position_y;
    float velocity_x;
    float velocity_y;
    int radius;
} Player;

int main() {

    Vector2 playerPosition = { WINDOWWIDTH/2.0f, WINDOWHEIGHT/2.0f };
    float BaseSpeed = 7.5f;
    float playerSpeed = BaseSpeed;
    float dodgeDistance = 10.0f;
    bool isDodging = false;
    double dodgeTime = 0.0f;
    double dodgeDuration = 0.2;

    InitWindow( WINDOWWIDTH, WINDOWHEIGHT, "Maze runner");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {

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

        //normalize and restrict movement to a strictly x and y axis without any xy movement.
        //if (Vector2Length(direction) != 0) direction = Vector2Normalize(direction);

        if (IsKeyPressed(KEY_SPACE) && Vector2Length(direction) > 0) {
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


        BeginDrawing();
        ClearBackground(BLACK);


        DrawCircleV(playerPosition, 12, RED);
        EndDrawing();
    }
    CloseWindow();

}