#include "raylib.h"
#include "raymath.h"
#include "MyCamera.h"
#include <iostream>
#include "BlockMesh.h"
#include <array>
#include <filesystem>
#include "TextureLoader.h"
#include "World.h"

const int SCWIDTH = 1280;
const int SCHEIGHT = 720;

void printMatrix(Matrix matrix) {
    std::cout << "[";
    std::cout << matrix.m0 << ", " << matrix.m4 << "," << matrix.m8 << "," << matrix.m12 << "," << std::endl;
    std::cout << matrix.m1 << ", " << matrix.m5 << "," << matrix.m9 << "," << matrix.m13 << "," << std::endl;
    std::cout << matrix.m2 << ", " << matrix.m6 << "," << matrix.m10 << "," << matrix.m14 << "," << std::endl;
    std::cout << matrix.m3 << ", " << matrix.m7 << "," << matrix.m11 << "," << matrix.m15 << "]" << std::endl;
}

void drawFPS() {
    std::string fps = std::to_string(GetFPS());
    DrawText(fps.c_str(), 0, 0, 40, BLACK);
}

void runGame() {
    TextureLoader::loadTextures();
    World world;
    world.update(0);

    // std::vector<std::unique_ptr<BlockMesh>> chunks;
    // for (int i = 0; i < 3; i++) {
    //     for (int k = 0; k < 3; k++) {
    //         chunks.push_back(std::make_unique<BlockMesh>(Vector3{16.0f*i, 0.0f, 16.0f*k}));
    //     }
    // }

    double time = GetTime();
    double dt;
    int moved = 0;
    SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
    ToggleFullscreen();
    DisableCursor();

    while (!WindowShouldClose()) {
        #ifndef __linux__
            if (!IsWindowFocused()) {
                SetWindowState(FLAG_WINDOW_MINIMIZED);
            }
            if (IsWindowFocused()) {
                if (!IsWindowFullscreen())
                    ToggleFullscreen();
            }
        #endif
        dt = GetTime() - time;
        time = GetTime();
        moved += 1;
        world.update(dt);
        SetMousePosition(SCWIDTH/2, SCHEIGHT/2);
        BeginDrawing();
        BeginMode3D(world.getPlayer());
		ClearBackground(WHITE);

        world.drawChunks();

        EndMode3D();
        drawFPS();
		EndDrawing();
    }

    TextureLoader::unloadTextures();
 
}

int main() {
    InitWindow(SCWIDTH, SCHEIGHT, "Minecraft");
    SetTraceLogLevel(LOG_WARNING);
    std::filesystem::current_path("../");
	
    // Loading Screen
    BeginDrawing();
    Image loadingScreen = LoadImage("assets/textures/sprites/minceraft.png");
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), LIGHTGRAY);
    Texture loadingTex = LoadTextureFromImage(loadingScreen);
    DrawTexture(loadingTex, 
                GetScreenWidth()/2 - loadingScreen.width/2, 
                GetScreenHeight()/2 - loadingScreen.height/2, 
                WHITE);
    EndDrawing();

    // Main game
    runGame();

    UnloadTexture(loadingTex);
    UnloadImage(loadingScreen);

    CloseWindow();
    return 0;
}