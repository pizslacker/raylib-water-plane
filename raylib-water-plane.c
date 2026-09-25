#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define GRID_SIZE 60
#define GRID_SPACING 0.5f

// Convert Amiga-style 0-255 colors into Raylib Colors
Color LerpColor(Color c1, Color c2, float amount) {
    return (Color){
        (unsigned char)(c1.r + (c2.r - c1.r) * amount),
        (unsigned char)(c1.g + (c2.g - c1.g) * amount),
        (unsigned char)(c1.b + (c2.b - c1.b) * amount),
        255
    };
}

int main(int argc, char **argv) {
    int screenWidth = 1280;
    int screenHeight = 1024;
    int targetFPS = 60;
    float speedMultiplier = 1.0f;
    bool fullscreen = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--width") == 0 || strcmp(argv[i], "-w") == 0) {
            if (i + 1 < argc) screenWidth = atoi(argv[++i]);
        } 
        else if (strcmp(argv[i], "--height") == 0 || strcmp(argv[i], "-h") == 0) {
            if (i + 1 < argc) screenHeight = atoi(argv[++i]);
        } 
        else if (strcmp(argv[i], "--fps") == 0 || strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) targetFPS = atoi(argv[++i]);
        } 
        else if (strcmp(argv[i], "--speed") == 0 || strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) speedMultiplier = atof(argv[++i]);
        } 
        else if (strcmp(argv[i], "--fullscreen") == 0) {
            fullscreen = true;
        } 
        else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("  -w, --width <pixels>    Set window width\n");
            printf("  -h, --height <pixels>   Set window height\n");
            printf("  -f, --fps <frames>      Set target FPS\n");
            printf("  -s, --speed <float>     Animation speed multiplier\n");
            printf("  --fullscreen            Enable fullscreen mode\n");
            return 0;
        }
    }

    // Initialize SDL2 specifically for Audio only (leaving video to Raylib)
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL Audio Init Failed: %s\n", SDL_GetError());
    } else {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            fprintf(stderr, "SDL_mixer Init Failed: %s\n", Mix_GetError());
        } else {
            Mix_Music *bgm = Mix_LoadMUS("bgm.mod");
            if (bgm) {
                Mix_PlayMusic(bgm, -1); // -1 triggers infinite loop
            } else {
                fprintf(stderr, "Could not load bgm.mod: %s\n", Mix_GetError());
            }
        }
    }

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Amiga Boing Ball Ripple Demo in C/SDL2/Raylib!");

    if (fullscreen) ToggleFullscreen();

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 12.0f, 18.0f };
    camera.target   = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy     = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Image checkedImg = GenImageChecked(512, 512, 16, 16, RED, WHITE);
    Texture2D boingTexture = LoadTextureFromImage(checkedImg);
    UnloadImage(checkedImg); 

    Mesh sphereMesh = GenMeshSphere(2.5f, 32, 32);
    Model boingBall = LoadModelFromMesh(sphereMesh);
    boingBall.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = boingTexture;

    SetTargetFPS(targetFPS);

    while (!WindowShouldClose()) {
        float time = (float)GetTime() * speedMultiplier;

        BeginDrawing();
            ClearBackground((Color){ 15, 15, 15, 255 }); 

            // Background: Simulated Amiga "Copper" raster gradient
            for (int y = 0; y < screenHeight; y++) {
                float norm = (float)y / screenHeight;
                Color topCol = (Color){ 5, 5, 20, 255 };
                Color botCol = (Color){ 20, 45, 90, 255 };
                DrawLine(0, y, screenWidth, y, LerpColor(topCol, botCol, norm));
            }

            BeginMode3D(camera);
                
                for (int z = -GRID_SIZE/2; z < GRID_SIZE/2; z++) {
                    for (int x = -GRID_SIZE/2; x < GRID_SIZE/2; x++) {
                        
                        float vx = x * GRID_SPACING;
                        float vz = z * GRID_SPACING;
                        float dist = sqrtf(vx * vx + vz * vz);
                        
                        float damping = fmaxf(0.0f, 1.0f - (dist / 14.0f));
                        float vy = sinf(dist * 2.0f - time * 5.0f) * 1.2f * damping;
                        Vector3 p1 = { vx, vy, vz };

                        Color gridColor = (Color){ 50, 180, 255, 255 }; 

                        if (x < GRID_SIZE/2 - 1) {
                            float n_vx = (x + 1) * GRID_SPACING;
                            float n_dist = sqrtf(n_vx * n_vx + vz * vz);
                            float n_damping = fmaxf(0.0f, 1.0f - (n_dist / 14.0f));
                            float n_vy = sinf(n_dist * 2.0f - time * 5.0f) * 1.2f * n_damping;
                            DrawLine3D(p1, (Vector3){ n_vx, n_vy, vz }, gridColor);
                        }
                        
                        if (z < GRID_SIZE/2 - 1) {
                            float n_vz = (z + 1) * GRID_SPACING;
                            float n_dist = sqrtf(vx * vx + n_vz * n_vz);
                            float n_damping = fmaxf(0.0f, 1.0f - (n_dist / 14.0f));
                            float n_vy = sinf(n_dist * 2.0f - time * 5.0f) * 1.2f * n_damping;
                            DrawLine3D(p1, (Vector3){ vx, n_vy, n_vz }, gridColor);
                        }
                    }
                }

                float ballY = 1.0f + fabsf(sinf(time * 3.5f)) * 6.0f;
                Vector3 ballPos = (Vector3){ 0.0f, ballY, 0.0f };
                
                Vector3 rotAxis = Vector3Normalize((Vector3){ 0.5f, 1.0f, -0.2f });
                float rotAngle = time * 40.0f; 

                DrawModelEx(boingBall, ballPos, rotAxis, rotAngle, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);

            EndMode3D();

            // Overlay Text
            DrawText("k!M", screenWidth/3 - 50, 40, 80, (Color){ 100, 150, 255, 255 });
            DrawText("2026", screenWidth/3 - 35, 120, 30, (Color){ 150, 200, 150, 255 });

            DrawText("DANK ROOM STUDIOS PRESENTS...", screenWidth/2 - 165, screenHeight - 150, 30, (Color){ 200, 100, 80, 255 });
            DrawText("BOING BALL DIMENSION...", screenWidth/2 - 130, screenHeight - 110, 30, (Color){ 100, 255, 100, 255 });
            DrawText("RIPPLE PLANE DEMO... AMIGA POWER!", screenWidth/2 - 195, screenHeight - 70, 30, (Color){ 50, 180, 255, 255 });

        EndDrawing();
    }

    UnloadTexture(boingTexture);
    UnloadModel(boingBall);
    CloseWindow();
    
    // Cleanup SDL Audio
    Mix_HaltMusic();
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    SDL_Quit();

    return 0;
}