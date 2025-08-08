#include "raylib.h"
#include "raymath.h"
#include "Player.h"
#include <iostream>
#include "BlockMesh.h"
#include <array>
#include <filesystem>
#include "TextureLoader.h"
#include "World.h"
#include "rlgl.h"
#include "Structure.h"

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

void drawFPS(Vector2 loc, const char* text, float fps) {
    std::string fpsText = text + std::to_string((int)round(fps));
    DrawText(fpsText.c_str(), loc.x, loc.y, 40, DARKGRAY);
}

bool AltF4Pressed() {
    return ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyDown(KEY_F4));
}

void runGame() {
    // Load the world
    TextureLoader::loadTextures();
    Structures::generateStructs();
    Image loadingImg = LoadImage("assets/textures/sprites/minceraft.png");
    Texture loadingTex = LoadTextureFromImage(loadingImg);
    World world;
    world.load(loadingTex);
    UnloadTexture(loadingTex);
    UnloadImage(loadingImg);
    double fps[300] = {0};
    int fpsIndex = 0;

    double time = GetTime();
    double dt;
    SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
    SetWindowPosition(0, 0);
    #ifndef DEBUG
        ToggleFullscreen();
        DisableCursor();
    #endif

    while (!AltF4Pressed() && !WindowShouldClose()) {
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
        ClearBackground(WHITE);
        BeginMode3D(world.getPlayer());
        // 3D rendering

        world.drawSkybox();
        world.drawChunks();
        world.getPlayer().draw3DElements();

        EndMode3D();
        // 2D rendering
        world.getPlayer().drawHud();

        fps[fpsIndex] = 1 / dt;
        fpsIndex = (fpsIndex + 1) % 300;
        drawFPS({0, 0}, "Min FPS: ", *std::min_element(fps, fps + 300));
        drawFPS({0, 40}, "FPS: ", GetFPS());
		EndDrawing();
    }

    TextureLoader::unloadTextures();
    SetWindowState(FLAG_WINDOW_HIDDEN);     // "Close" the window instantly, while still allowing for destructors and other cleanup
    SetWindowState(FLAG_WINDOW_MINIMIZED);
}

int main() {
    InitWindow(SCWIDTH, SCHEIGHT, "Minecraft");
    rlSetLineWidth(GetMonitorWidth(GetCurrentMonitor())/1280.0);
    rlEnableSmoothLines();
    SetExitKey(-1);
    SetTraceLogLevel(LOG_WARNING);
    std::filesystem::current_path(ROOT_PATH);

    // Main game
    runGame();

    CloseWindow();
    return 0;
}