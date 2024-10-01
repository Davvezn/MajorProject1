#include "raylib.h"
#include <raymath.h>

#define WINDOWWIDTH 800
#define WINDOWHEIGHT 600

#define PLAYER_MAX_ARROWS 500
#define MAX_MELEES 500
#define MAX_REDS 90
#define MAX_BLUES 100
#define MAX_PURPLES 50

#define MAX_ZOMBIES 80

typedef struct Bullet {
    Vector2 position;
    Vector2 speed;
    bool active;
    Color color;
} Projectile;

typedef struct Melee {
    Vector2 position;
    Vector2 direction;
    Vector2 size;
    bool active;
    Color color;
} melee;

typedef struct Red {
    Vector2 position;
    Vector2 speed;
    bool active;
    int bounces;
}Red;

typedef struct Enemies {
    int HP;
    int Attack;
    Vector2 position;
    Vector2 speed;
    Color color;
    bool active; 
} enemies;

Vector2 getRandomSpawnLocation();

void spawnZombies(enemies Zombie[], Vector2 position, Vector2 direction, int zombiesToSpawn) {
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!Zombie[i].active) {
            Vector2 randomSpawnLocation = getRandomSpawnLocation();
            Vector2 randomDirection = Vector2Normalize((Vector2){GetRandomValue(-1,1), GetRandomValue(-1,1)});

            Zombie[i].position = randomSpawnLocation;
            Zombie[i].speed = Vector2Scale(randomDirection, 6.0f);
            Zombie[i].HP = 100;
            Zombie[i].active = true;

            zombiesToSpawn--;
        }
        if (zombiesToSpawn <= 0) {
            break;
        }
    }
}

void updateZombies(enemies Zombie[], Vector2 playerPosition, float zombieSpeed, Projectile arrows[], Red red[]) {
    for ( int i = 0; i < MAX_ZOMBIES; i++) {
        if (Zombie[i].active) {
            //logic about zombie movement etc
            Vector2 direction = Vector2Subtract(playerPosition, Zombie[i].position);

            Vector2 normalizedDirection = Vector2Normalize(direction);

            Zombie[i].position.x += normalizedDirection.x * zombieSpeed * GetFrameTime();
            Zombie[i].position.y += normalizedDirection.y * zombieSpeed * GetFrameTime();


            if (Zombie[i].position.x < 0 + 10) {
                Zombie[i].position.x = 0 + 10;
            }
            if (Zombie[i].position.x >  WINDOWWIDTH - 10) {
                Zombie[i].position.x = WINDOWWIDTH - 10;
            }
            if (Zombie[i].position.y < 0 + 10) {
                Zombie[i].position.y = 0 + 10;
            }
            if (Zombie[i].position.y > WINDOWHEIGHT - 10) {
                Zombie[i].position.y =  WINDOWHEIGHT - 10;
            }
            for (int j = 0; j < PLAYER_MAX_ARROWS; j++) {
                if (arrows[j].active && CheckCollisionCircles(Zombie[i].position, 20, arrows[j].position, 10)) {
                    Zombie[i].HP -= 10;
                    arrows[j].active = false;
                }
            }

            for (int r = 0; r < MAX_REDS; r++) {
                if (red[r].active && CheckCollisionCircles(red[r].position, 40, Zombie[i].position, 20)) {
                    Zombie[i].HP -= 200;
                }
            }

            // hp check
            if (Zombie[i].HP <=0) {
                Zombie[i].active = false;
            }

        }
    }
}

void drawZombie(enemies Zombie[]) {
    for (int i =0; i < MAX_ZOMBIES; i++) {
        if (Zombie[i].active) {
            DrawCircleV(Zombie[i].position, 10, GREEN);
        }
    }
}

void shootArrow(Projectile arrows[],Vector2 position, Vector2 direction) {
    for (int i = 0; i < PLAYER_MAX_ARROWS; i++) {
        if (!arrows[i].active) {
            arrows[i].position = position;
            arrows[i].speed = direction;
            arrows[i].active = true;
            break;
        }
    }
}

void updateArrow(Projectile arrows[]) {
    for (int i = 0; i < PLAYER_MAX_ARROWS; i++) {
        if (arrows[i].active) {
            arrows[i].position.x += arrows[i].speed.x;
            arrows[i].position.y += arrows[i].speed.y;

            if (arrows[i].position.x < 0 || arrows[i].position.x > WINDOWWIDTH || arrows[i].position.y < 0 || arrows[i].position.y > WINDOWHEIGHT) { //if outside window
                arrows[i].active = false;
            }
        }
    }
}

void DrawArrow(Projectile arrows[]) {
    for (int i = 0; i< PLAYER_MAX_ARROWS; i++) {
        if (arrows[i].active) {
            DrawCircleV(arrows[i].position, 5, PURPLE);
        }
    }
}

void ShootRed(Red red[], Vector2 position, Vector2 direction){
    for (int i = 0; i< MAX_REDS; i++) {
        if (!red[i].active) {
            red[i].position = position;
            red[i].speed = Vector2Scale(Vector2Normalize(direction), 10.0f);
            red[i].active = true;
            red[i].bounces = 0;
            break;
        }
    }
}

void updateRed(Red red[]) {
    for (int i = 0; i < MAX_REDS; i++) {
        if (red[i].active == true) {
            red[i].position.x += red[i].speed.x;
            red[i].position.y += red[i].speed.y;

            
            if (red[i].position.x < 10 || red[i].position.x > WINDOWWIDTH - 10) {
                 red[i].speed.x *= -1;
                red[i].bounces++;
            }

            if (red[i].position.y < 10 || red[i].position.y > WINDOWHEIGHT - 10) {
                red[i].speed.y *= -1;
                red[i].bounces++;
            }
            
            if (red[i].bounces >= 4) {
                red[i].active = false;
            }
        }
    }
} 

void DrawRed(Red red[]) {
    for (int i = 0; i < MAX_REDS; i++) {
        if(red[i].active) {
            DrawCircleV(red[i].position, 20, RED);
        }
    }
}

void MeleeAttack(melee melees[], Vector2 position, Vector2 direction, Vector2 size/*might be able to add enemies etc here later*/) {
    for ( int i = 0; i < MAX_MELEES; i++) {
        if (!melees[i].active) {

            Vector2 normalizedDirection = Vector2Normalize(direction);

            melees[i].position.x = position.x + normalizedDirection.x * (size.y / 3 + 1);
            melees[i].position.y = position.y + normalizedDirection.y * (size.y / 3 + 1);

            melees[i].direction = direction;  
            melees[i].active = true;
            melees[i].size = size;
            break;
        }
    }
}

void updateAttack(melee melees[]) {
    for (int i = 0; i < MAX_MELEES; i++) {
        if (melees[i].active) {

        }
    }
}

void DrawAttackAoe(melee melees[]) {
    for (int i = 0; i < MAX_MELEES; i++) {
        if (melees[i].active) {
            Rectangle meleeArea = { melees[i].position.x, melees[i].position.y, melees[i].size.x, melees[i].size.y};

            Vector2 origin = {melees[i].size.x / 2.0f, melees[i].size.y / 2.0f};

            float rotation = atan2f(melees[i].direction.y, melees[i].direction.x) * (180.0f /PI);
            DrawRectanglePro(meleeArea, origin, rotation, BLUE);
        }
    }
}

Vector2 getRandomSpawnLocation() {
    Vector2 spawnPosition;
    int edge = GetRandomValue(0, 3);

    switch (edge) {
        case 0:
            spawnPosition.x = 0;
            spawnPosition.y = GetRandomValue(0, WINDOWHEIGHT);
            break;
        case 1:
            spawnPosition.x = WINDOWWIDTH;
            spawnPosition.y = GetRandomValue(0, WINDOWHEIGHT);
            break;
        case 2:
            spawnPosition.x = GetRandomValue(0, WINDOWWIDTH);
            spawnPosition.y = 0;
            break;
        case 3:
            spawnPosition.x = GetRandomValue(0, WINDOWWIDTH );
            spawnPosition.y = WINDOWHEIGHT;
    }   
    return spawnPosition;
}

int main() {

    Vector2 playerPosition = { WINDOWWIDTH/2.0f, WINDOWHEIGHT/2.0f };
    float BaseSpeed = 7.5f;
    float playerSpeed = BaseSpeed;

    // dodge values
    float dodgeDistance = 10.0f;
    bool isDodging = false;
    double dodgeTime = 0.2f;
    double dodgeDuration = 0.2;

    // attack logics
    float meleeDuration = 0.5f;

    // ability defines
    Color StaminaBarColor = {37, 206, 209, 100};
    float lastRed = -10.0;
    float redCooldownTime = 2.0; //should be 10s

    // enemie defines / logic initiation
    float Zombie_Speed = 40.0f;
    float Spawntime = 0.0f;
    float Spawninterval = 5.0f;
    int zombiesToSpawn = 10;
    int ActiveZombies = 0;

    melee melees[MAX_MELEES] = {0};

    Projectile arrows[PLAYER_MAX_ARROWS] = {0};

    Red red[MAX_REDS] = {0};
    enemies Zombie[MAX_ZOMBIES] = {0};

    InitWindow( WINDOWWIDTH, WINDOWHEIGHT, "Maze runner");
    SetTargetFPS(60);


    while (!WindowShouldClose()) {
        Vector2 mousePosition = GetMousePosition();
        Vector2 BulletDirection = Vector2Normalize(Vector2Subtract(mousePosition, playerPosition));
        Vector2 arrowSpeed = {BulletDirection.x * 10.0f, BulletDirection.y * 10.0};
        Vector2 MeleeDirection = Vector2Normalize(Vector2Subtract(mousePosition, playerPosition));

        if (zombiesToSpawn >= 0) {
            spawnZombies(Zombie, playerPosition, Vector2Zero(), zombiesToSpawn);
            zombiesToSpawn = 1;
        }
        //updates
        updateArrow(arrows);
        updateRed(red);
        updateZombies(Zombie, playerPosition, Zombie_Speed, arrows, red);

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

        if (IsKeyReleased(KEY_E)) {
            if (currentTime >= lastRed + redCooldownTime) {
                ShootRed(red, playerPosition, arrowSpeed);
                lastRed = currentTime;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 meleeSize = { 30.0f, 70.0f}; // size of melee attack
            MeleeAttack(melees, playerPosition, MeleeDirection, meleeSize);
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
        // Schematic for stamina / mana bar
        float cooldownPercentage = (currentTime - lastRed) / redCooldownTime;
        if (cooldownPercentage > 1.0f) cooldownPercentage = 1.0f; // Cap it at 1 (100%)


        BeginDrawing();
        ClearBackground(BLACK);

        drawZombie(Zombie);
        DrawAttackAoe(melees);
        DrawCircleV(playerPosition, 12, WHITE); // player
        DrawArrow(arrows);
        DrawRed(red);
        DrawRectangle(50, 50, 20, (int)(200 * cooldownPercentage), StaminaBarColor); 
        EndDrawing();
    }

    CloseWindow();
}