#include "raylib.h"
#include <raymath.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    float x;
    float y;
    float w;
    float h;
    float rot;
} rotated_rect_t;

int total_exp_earn = 0;
int spawnedZombies = 0;
float timeBetweenSpawns = 0.5f;
float spawnTimerZombies = 0.0f;
float spawnTimerOrcs = 0.0f;
int zombiesToSpawn = 0;
int orcs_to_spawn = 0;
int spawnedOrcs = 0;
bool melee_active = false;

#define WINDOWWIDTH 800
#define WINDOWHEIGHT 600

#define PLAYER_MAX_ARROWS 500
#define MAX_MELEES 500
#define MAX_REDS 10
#define MAX_BLUES 10 //future abilities
#define MAX_PURPLES 5

#define MAX_ZOMBIES 100
#define MAX_ORCS 20

typedef struct Player {
    float HP;
    bool active;
    Vector2 position;
    Vector2 speed;
    Color color;
} Player;

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
    float lifetime;
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
    int exp_on_kill;
    bool active; 
} enemies;

void DrawRotatedRectangle(rotated_rect_t rect, Color color) {
    Vector2 origin = { rect.x + rect.w / 2.0f, rect.y + rect.h / 2.0f};
    DrawRectanglePro((Rectangle){ rect.x, rect.y, rect.w, rect.h}, origin, rect.rot, color); // temporary to make a mindmap
}

bool IsCircleInRotatedRect(Vector2 circlePos, float radius, rotated_rect_t rect) {
    Vector2 localCirclePos = Vector2Subtract(circlePos, (Vector2){ rect.x + rect.w / 2.0f, rect.y + rect.h / 2.0f });

    float cosine = cosf(-rect.rot * (PI / 180.0f));
    float sine = sinf(-rect.rot * (PI / 180.0f));

    float rotatedX = localCirclePos.x * cosine - localCirclePos.y * sine;
    float rotatedY = localCirclePos.y * sine -localCirclePos.y * cosine;

    float halfWidth = rect.w / 2.0;
    float halfHeight = rect.h / 2.0f;

    float closestX = fmax(-halfWidth, fmin(rotatedX, halfWidth));
    float closestY = fmax(-halfHeight, fmin(rotatedY, halfHeight));

    float distanceX = rotatedX - closestX;
    float distanceY = rotatedY - closestY;

    return (distanceX * distanceX + distanceY * distanceY) <= (radius * radius); 
}

Vector2 getRandomSpawnLocation();

void updateExp(int exp) {
    total_exp_earn += exp;
}

void damageEnemies(enemies *enemies, int playerDamage) {
    enemies->HP -= playerDamage;

    if (enemies->HP <= 0) {
        enemies->active = false;
        int enemyXP = enemies->exp_on_kill;
        //printf("xp earned: %d\n", enemyXP);
        total_exp_earn += enemyXP;
        enemyXP = 0;
        //printf("total exp obtained: %d\n", total_exp_earn);
    }
}

int player_level = 1;
int exp_to_next_lvl = 100; // 10 zombies


void checkLevelUp() {
    if (total_exp_earn >= exp_to_next_lvl) {
        player_level++;
        total_exp_earn -= exp_to_next_lvl;
        exp_to_next_lvl += 100; // turn into exponential growth later
    }
}

void spawnOrc(enemies Orcs[], Vector2 position, Vector2 direction, int orcs_to_spawn, int *spawnedOrcs) {
    for (int i = 0; i < MAX_ORCS && *spawnedOrcs < orcs_to_spawn; i++) {
        if (!Orcs[i].active) {
            if (*spawnedOrcs < orcs_to_spawn) {
                Vector2 randomSpawnLocation = getRandomSpawnLocation();
                Vector2 randomDirection = Vector2Normalize((Vector2){GetRandomValue(-1,1), GetRandomValue(-1,1)});

                Color orc_color = (Color){44, 105, 58, 255};

                Orcs[i].position = randomSpawnLocation;
                Orcs[i].speed = Vector2Scale(randomDirection, 6.0f);
                Orcs[i].HP = 1000;
                Orcs[i].active = true;
                Orcs[i].color = orc_color;
                Orcs[i].exp_on_kill = 1000;

                (*spawnedOrcs)++;
            } else {
                break;
            }
        }
    }
}

void updateOrcs(enemies Orcs[],Vector2 playerPosition, Player player_one, Red red[], Projectile arrows[], int orcs_to_spawn, int *spawnedOrcs) {
    for (int i = 0; i < MAX_ORCS; i++) {
        if (Orcs[i].active) {
            Vector2 orcDirection = Vector2Subtract(playerPosition, Orcs[i].position); //minimizes vector between orc pos and player pos.
            Vector2 orcNormalizedDirection = Vector2Normalize(orcDirection);

            Vector2 knockback_direction = Vector2Normalize(Vector2Subtract(Orcs[i].position, red[i].position));
            Vector2 bounce_direction = Vector2Negate(knockback_direction);
            float knockback_Strength = 20.0f;

            int orc_speed = 20;

            Orcs[i].position.x += orcNormalizedDirection.x * orc_speed * GetFrameTime();
            Orcs[i].position.y += orcNormalizedDirection.y * orc_speed * GetFrameTime();

            //border collision 
            if (Orcs[i].position.x > WINDOWWIDTH - 20) {
                Orcs[i].position.x = WINDOWWIDTH - 20;
            }
            if (Orcs[i].position.x < 0 + 20) {
                Orcs[i].position.x = 0 + 20;
            }
            if (Orcs[i].position.y < 0 + 20) {
                Orcs[i].position.y = 0 + 20;
            }
            if (Orcs[i].position.y > WINDOWHEIGHT - 20) {
                Orcs[i].position.y = WINDOWHEIGHT - 20;
            }

            if (CheckCollisionCircles(Orcs[i].position, 20, playerPosition, 12)) {
                player_one.HP -= 20;

                player_one.HP = Clamp(player_one.HP, 0, 100);

                if (player_one.HP <= 0) {
                    player_one.active = false; 
                }
            }

            for (int r = 0; r < MAX_REDS; r++) {
                if (red[r].active && CheckCollisionCircles(Orcs[i].position, 29, red[r].position, 20)) {
                    damageEnemies(&Orcs[i], 100 /*potential hard coding could happen here */);


                    if (Orcs[i].HP > 0) { // makes sure orcs dont bounce when dead
                        Orcs[i].position = Vector2Add(Orcs[i].position, Vector2Scale(bounce_direction, knockback_Strength));
                    }

                }
            }

            for (int a = 0; a < PLAYER_MAX_ARROWS; a++) {
                if (arrows[a].active && CheckCollisionCircles(Orcs[i].position, 40, arrows[a].position, 20)) {
                    damageEnemies(&Orcs[i], 10);

                    arrows[a].active = false;
                }

            }

            if (Orcs[i].HP <= 500) {
                Color orc_low_HP = (Color){186, 22, 22, 255};
                Orcs[i].color = orc_low_HP;
            }

            if (Orcs[i].HP <= 0) {
                Orcs[i].active = false;
                (*spawnedOrcs)--;
            }
        }
    }
    if (*spawnedOrcs < orcs_to_spawn){
        spawnTimerOrcs += GetFrameTime();

        if (spawnTimerOrcs >= timeBetweenSpawns) {
            Vector2 RandomSpawnLocation = getRandomSpawnLocation();
            spawnOrc(Orcs, RandomSpawnLocation, Vector2Zero(), orcs_to_spawn, spawnedOrcs);

            spawnTimerOrcs = 0.0f;
        }
    }
}

void drawOrcs(enemies Orcs[]) {
    for (int i = 0; i< MAX_ORCS; i++) {
        if (Orcs[i].active) {
            DrawCircleV(Orcs[i].position, 40, Orcs[i].color);
        }
    }
}

void spawnZombies(enemies Zombie[], Vector2 position, Vector2 direction, int zombiesToSpawn, int *spawnedZombies) {
    for (int i = 0; i < MAX_ZOMBIES && *spawnedZombies < zombiesToSpawn; i++) {
        if (!Zombie[i].active) {

            Vector2 randomSpawnLocation = getRandomSpawnLocation();
            Vector2 randomDirection = Vector2Normalize((Vector2){GetRandomValue(-1,1), GetRandomValue(-1,1)});

            Zombie[i].position = randomSpawnLocation;
            Zombie[i].speed = Vector2Scale(randomDirection, 6.0f);
            Zombie[i].HP = 100;
            Zombie[i].active = true;
            Zombie[i].exp_on_kill = 10;

            (*spawnedZombies)++;
        }
    }
}

void updateZombies(enemies Zombie[], Vector2 playerPosition, float zombieSpeed, Projectile arrows[], Red red[], Player player_one, int zombiesToSpawn, int *spawnedZombies) {
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
            for (int j = 0; j < MAX_ZOMBIES; j++) {
                if (i != j) {
                    if (CheckCollisionCircles(Zombie[i].position, 10, Zombie[j].position, 10)) { //self collision (kinda works)
                        Vector2 direction = Vector2Subtract(Zombie[i].position, Zombie[i].position);
                        float distance = Vector2Length(direction);

                        if (distance > 0){
                            direction = Vector2Scale(direction, 1.0f / distance);
                        }

                        float push_distance = 20 + 20 + 20 - distance;
                        Zombie[i].position = Vector2Add(Zombie[i].position, Vector2Scale(direction, push_distance));
                    }
                }
            }

            if (CheckCollisionCircles(Zombie[i].position, 20, playerPosition, 12)) {
                player_one.HP -= 5;

                player_one.HP = Clamp(player_one.HP, 0, 100);

                if (player_one.HP <= 0) {
                    player_one.active = false; 
                }
            }


            for (int j = 0; j < PLAYER_MAX_ARROWS; j++) {
                if (arrows[j].active && CheckCollisionCircles(Zombie[i].position, 20, arrows[j].position, 5)) {
                    damageEnemies(&Zombie[i], 20 + 1.5 * player_level);

                    arrows[j].active = false;
                }
            }

            for (int r = 0; r < MAX_REDS; r++) {
                if (red[r].active && CheckCollisionCircles(red[r].position, 20, Zombie[i].position, 20)) {
                    damageEnemies(&Zombie[i], 200 + 1.01 * player_level);
                }
            }

            if (Zombie[i].HP <= 0) {
                Zombie[i].active = false;
                (*spawnedZombies)--;
            }

        }
    }
    if (*spawnedZombies < zombiesToSpawn){
        spawnTimerZombies += GetFrameTime();

        if (spawnTimerZombies >= timeBetweenSpawns) {
            Vector2 RandomSpawnLocation = getRandomSpawnLocation();
            spawnZombies(Zombie, RandomSpawnLocation, Vector2Zero(), zombiesToSpawn/*only accounts and respawns set amount*/, spawnedZombies);

            spawnTimerZombies = 0.0f;
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
            
            melees[i].lifetime = 0.2;
            melees[i].direction = direction;  
            melees[i].active = true;
            melees[i].size = size;
            /*
            DrawRectangle(melees[i].position.x - melees[i].size.x / 2.0f,
                melees[i].position.y - melees[i].size.y / 2.0f,
                melees[i].size.x,
                melees[i].size.y,
                YELLOW);*/
            break;
        }
    }
}

void updateAttack(melee melees[], enemies Zombie[], enemies Orcs[]) {
    for (int i = 0; i < MAX_MELEES; i++) {
        if (melees[i].active) {
            float rotation = atan2f(melees[i].direction.y, melees[i].direction.x) * (180.0f / PI);
            rotated_rect_t meleeRect = {
                .x = melees[i].position.x,
                .y = melees[i].position.y,
                .w = melees[i].size.x,
                .h = melees[i].size.y,
                .rot = rotation
            };
            melees[i].lifetime -= GetFrameTime();

            if (melees[i].lifetime <= 0.0) {
                melees[i].active = false;
            }
            

            for (int z = 0; z < MAX_ZOMBIES; z++) {
                if (IsCircleInRotatedRect(Zombie[z].position, 20, meleeRect)) {
                    damageEnemies(&Zombie[z], 50 + 1.01 * player_level);
                }
            }

            for (int O = 0; O < MAX_ORCS; O++) {
                if (Orcs[O].active && IsCircleInRotatedRect(Orcs[O].position, 40, meleeRect)) {
                    damageEnemies(&Orcs[O], 50 + 1.01 * player_level);
                }
            }
        }
    }
}

void DrawAttackAoe(melee melees[]) {
    for (int i = 0; i < MAX_MELEES; i++) {
        if (melees[i].active) {
            float rotation = atan2f(melees[i].direction.y, melees[i].direction.x) * (180.0f /PI);
            Rectangle meleeArea = { melees[i].position.x, melees[i].position.y, melees[i].size.x, melees[i].size.y};
            Vector2 origin = {melees[i].size.x / 2.0f, melees[i].size.y / 2.0f};
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
    bool gameOver = false;

    Player player_one = {100, true, { WINDOWWIDTH/2, WINDOWHEIGHT/2}, {7.5f,7.5f}, WHITE};
    int max_hp = 100;
    Color back_ground_bar = {79,79,79, 155};
    Color hp_bar_color ={5, 181, 41, 155};
    float BaseSpeed = 7.5f;
    float SprintMultiplier = 1.5f;

    // dodge values
    float dodgeDistance = 10.0f;
    bool isDodging = false;
    double dodgeTime = 0.2f;
    double dodgeDuration = 0.2;

    // attack logics
    float meleeDuration = 0.5f;

    // ability defines
    Color ManaBarColor = {201, 0, 20, 100};
    float lastRed = -10.0;
    float redCooldownTime = 10.0; //should be 10s

    // enemie defines / logic initiation
    float Zombie_Speed = 40.0f;
    float Spawntime = 0.0f;
    float Spawninterval = 5.0f;
    int ActiveZombies = 0;
    float damageCooldown = 1.0f;
    float last_damage_tick = -1.0f;
    int lastSpawnTime = 0;

    melee melees[MAX_MELEES] = {0};
    Projectile arrows[PLAYER_MAX_ARROWS] = {0};
    Red red[MAX_REDS] = {0};
    enemies Zombie[MAX_ZOMBIES] = {0};
    enemies Orcs[MAX_ORCS] = {0};

    InitWindow( WINDOWWIDTH, WINDOWHEIGHT, "Maze runner");
    SetTargetFPS(60);
    while (!WindowShouldClose() && !gameOver) {
        Vector2 mousePosition = GetMousePosition();
        Vector2 BulletDirection = Vector2Normalize(Vector2Subtract(mousePosition, player_one.position));
        Vector2 arrowSpeed = {BulletDirection.x * 10.0f, BulletDirection.y * 10.0};
        Vector2 MeleeDirection = Vector2Normalize(Vector2Subtract(mousePosition, player_one.position));
        double total_runtime = GetTime();

        if (total_runtime >= 1 && lastSpawnTime < 1) {
            zombiesToSpawn = 5;
            
            spawnZombies(Zombie, player_one.position, Vector2Zero(), zombiesToSpawn, &spawnedZombies);
            lastSpawnTime = total_runtime;
        }

        if (total_runtime >= 10 && lastSpawnTime < 10) {

            zombiesToSpawn = 15; // only spawns 15 but doesnt allow 15 to be respawned
            orcs_to_spawn = 1;

            spawnOrc(Orcs, player_one.position, Vector2Zero(), orcs_to_spawn, &spawnedOrcs);
            spawnZombies(Zombie, player_one.position, Vector2Zero(), zombiesToSpawn, &spawnedZombies);
            lastSpawnTime = total_runtime;
        }

        if ( total_runtime >= 60 && lastSpawnTime < 60) {
            zombiesToSpawn = 25;
            orcs_to_spawn = 3;

            spawnOrc(Orcs, player_one.position, Vector2Zero(), orcs_to_spawn, &spawnedOrcs);
            spawnZombies(Zombie, player_one.position, Vector2Zero(), zombiesToSpawn, &spawnedZombies);

            lastSpawnTime = total_runtime;
            // enable red
            // max zombie -> 25
            // orcs -> 3
        }

        if (total_runtime >= 120) {
            zombiesToSpawn = 50;
            orcs_to_spawn = 5;

            spawnOrc(Orcs, player_one.position, Vector2Zero(), orcs_to_spawn, &spawnedOrcs);
            spawnZombies(Zombie, player_one.position, Vector2Zero(), zombiesToSpawn, &spawnedZombies);

        }

        if (total_runtime >= 150) {
            zombiesToSpawn = 75;
            orcs_to_spawn = 15;
            // disable melee
            spawnOrc(Orcs, player_one.position, Vector2Zero(), orcs_to_spawn, &spawnedOrcs);
            spawnZombies(Zombie, player_one.position, Vector2Zero(), zombiesToSpawn, &spawnedZombies);
        }

        if (total_runtime >= 240) {
            zombiesToSpawn = 100;
            orcs_to_spawn = 20;
            melee_active = true;

            spawnOrc(Orcs, player_one.position, Vector2Zero(), orcs_to_spawn, &spawnedOrcs);
            spawnZombies(Zombie, player_one.position, Vector2Zero(), zombiesToSpawn, &spawnedZombies);
        }




        //updates
        updateAttack(melees, Zombie, Orcs);
        updateArrow(arrows);
        updateRed(red);
        updateZombies(Zombie, player_one.position, Zombie_Speed, arrows, red, player_one, zombiesToSpawn, &spawnedZombies);
        updateOrcs(Orcs, player_one.position, player_one, red, arrows, orcs_to_spawn, &spawnedOrcs);
        checkLevelUp();

        double currentTime = GetTime();

        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            player_one.speed.x = BaseSpeed * SprintMultiplier;
            player_one.speed.y = BaseSpeed * SprintMultiplier;
        } else {
            player_one.speed.x = BaseSpeed;
            player_one.speed.y = BaseSpeed;
        }

        Vector2 direction = {0.0f, 0.0f};
        //movement logic for player_one
        if(IsKeyDown(KEY_W)) {
            direction.y -= player_one.speed.y;
        }
        if(IsKeyDown(KEY_S)) {
            direction.y += player_one.speed.y;
        }
        if(IsKeyDown(KEY_A)) {
            direction.x -= player_one.speed.x;
        }
        if(IsKeyDown(KEY_D)) {
            direction.x += player_one.speed.x;
        }

        if(IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
            shootArrow(arrows, player_one.position, arrowSpeed);
        }

        if (IsKeyReleased(KEY_E)) {
            if (currentTime >= lastRed + redCooldownTime) {
                ShootRed(red, player_one.position, arrowSpeed);
                lastRed = currentTime;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (melee_active) {
                Vector2 meleeSize = { 30.0f, 70.0f};
                MeleeAttack(melees, player_one.position, MeleeDirection, meleeSize);
            }
        }

        if (IsKeyPressed(KEY_L)) {
            player_one.HP -= 10;
        }

        if (IsKeyPressed(KEY_ESCAPE)) { //simple exit funtion on esc
            CloseWindow();
        }

        // dodge logic
        if (IsKeyPressed(KEY_SPACE) && Vector2Length(direction) > 0) { // checks if space is active addiotinally if any other direction input is being used
            isDodging = true;
            dodgeTime = currentTime;
            player_one.position.x += direction.x * dodgeDistance;
            player_one.position.y += direction.y * dodgeDistance;
        }
        if (isDodging && (currentTime - dodgeTime) > dodgeDuration) {
            isDodging = false;
        }
        if (!isDodging) {
            player_one.position.x += direction.x * player_one.speed.x * GetFrameTime();
            player_one.position.y += direction.y * player_one.speed.y * GetFrameTime();
        }

        // border collision check
        if (player_one.position.x < 0 + 12) {
            player_one.position.x = 0 + 12;
        }
        if (player_one.position.x >  WINDOWWIDTH - 12) {
            player_one.position.x = WINDOWWIDTH - 12;
        }
        if (player_one.position.y < 0 + 12) {
            player_one.position.y = 0 + 12;
        }
        if (player_one.position.y > WINDOWHEIGHT - 12) {
            player_one.position.y =  WINDOWHEIGHT - 12;
        }

        // Schematic for stamina / mana bar
        float cooldownPercentage = (currentTime - lastRed) / redCooldownTime;
        if (cooldownPercentage > 1.0f) cooldownPercentage = 1.0f; // Cap it at 1 (100%)

        float healthPrecent = (float)player_one.HP / max_hp;

        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (Zombie[i].active && CheckCollisionCircles(Zombie[i].position, 20, player_one.position, 12)) {
                
                if (currentTime >= last_damage_tick + damageCooldown) {
                    player_one.HP -= 5;
                    last_damage_tick = currentTime;
                } 
                
                if (player_one.HP <= 0) {
                    gameOver = true;
                }
            }
        }

        BeginDrawing();

        if (gameOver) {
            DrawText("message", WINDOWWIDTH / 2 - 50, WINDOWHEIGHT / 2, 12, RED);
        } else {
            ClearBackground(BLACK);
            DrawAttackAoe(melees);
            drawZombie(Zombie);
            drawOrcs(Orcs);
            DrawCircleV(player_one.position, 12, WHITE); // player
            DrawArrow(arrows);
            DrawRed(red);
            DrawRectangle(10, 65, (int)(200 * cooldownPercentage), 20, ManaBarColor);
            DrawRectangle(10, 10,400,40, back_ground_bar);
            DrawRectangle(10, 10, (int)(400 * healthPrecent), 40, hp_bar_color);
            DrawText(TextFormat("Total Exp Earned: %d", total_exp_earn), 10, 100, 20, WHITE);
            DrawText(TextFormat("Current Level: %d", player_level), 10, 120, 20, WHITE);
            DrawText(TextFormat("Total Runtime: %.2f seconds", total_runtime), WINDOWWIDTH/2 - 40, WINDOWHEIGHT - 12, 12, WHITE);
        }

        EndDrawing();
    }
    CloseWindow();
}