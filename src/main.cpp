#include "raylib.h"
#include "raymath.h"
#include "Player.h"
#include <iostream>
#include "BlockMesh.h"
#include <array>
#include <filesystem>
#include "TextureLoader.h"
#include "World.h"

const int SCWIDTH = 1280;
const int SCHEIGHT = 720;

#define DEBUG

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
    // Load the world
    TextureLoader::loadTextures();
    Image loadingImg = LoadImage("assets/textures/sprites/minceraft.png");
    Texture loadingTex = LoadTextureFromImage(loadingImg);
    World world;
    world.load(loadingTex);
    UnloadTexture(loadingTex);
    UnloadImage(loadingImg);

    double time = GetTime();
    double dt;
    SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
    SetWindowPosition(0, 0);
    #ifndef DEBUG
        ToggleFullscreen();
        DisableCursor();
    #endif

    while (!WindowShouldClose()) {
        // Solves alt+tab issue on windows
        #ifdef _WIN32
            if (!IsWindowFocused() && IsWindowFullscreen()) {
                MinimizeWindow();
            }
            if (IsWindowFocused() && !IsWindowFullscreen()) {
                #ifndef DEBUG
                    ToggleFullscreen();
                #endif
            }
        #endif
        // Game update logic
        dt = GetTime() - time;
        time = GetTime();
                
        world.update(dt);
        SetMousePosition(GetMonitorWidth(GetCurrentMonitor())/2, GetMonitorHeight(GetCurrentMonitor())/2);
        
        // Drawing to screen
        BeginDrawing();
        BeginMode3D(world.getPlayer());
		ClearBackground(WHITE);
        // 3D rendering
        world.drawChunks();

        EndMode3D();
        // 2D rendering
        world.getPlayer().drawHud();
        drawFPS();

		EndDrawing();
    }

    TextureLoader::unloadTextures();
 
}

int main() {
    InitWindow(SCWIDTH, SCHEIGHT, "Minecraft");
    // SetTraceLogLevel(LOG_WARNING);
    std::filesystem::current_path("../");

    // Main game
    runGame();

    CloseWindow();
    return 0;
}