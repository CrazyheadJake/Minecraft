#include "raylib.h"
#include "raymath.h"
#include "MyCamera.h"
#include <iostream>
#include "BlockMesh.h"
#include <array>
#include <filesystem>
#include "TextureLoader.h"

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
    MyCamera camera({0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, 90.0f, CAMERA_PERSPECTIVE);

    TextureLoader::loadTextures();
    std::vector<std::unique_ptr<BlockMesh>> chunks;
    for (int i = 0; i < 3; i++) {
        for (int k = 0; k < 3; k++) {
            chunks.push_back(std::make_unique<BlockMesh>(Vector3{16.0f*i, 0.0f, 16.0f*k}));
        }
    }

    double time = GetTime();
    double dt;
    int moved = 0;
    ToggleFullscreen();
    DisableCursor();

    while (!WindowShouldClose()) {

        dt = GetTime() - time;
        time = GetTime();
        // std::cout << "DT: " << dt << std::endl;
        // SetWindowFocused();

        Vector2 dXY = GetMouseDelta();
        // std::cout << dXY.x << ", " << dXY.y << std::endl;
        moved += 1;
        SetMousePosition(SCWIDTH/2, SCHEIGHT/2);
        camera.changeYaw(dXY.x * camera.rotateSpeed);
        camera.changePitch(dXY.y * camera.rotateSpeed);
        camera.update();
        
        if (IsKeyDown(KEY_W)) {
            camera.moveFwd(dt);
        }
        if (IsKeyDown(KEY_S)) {
            camera.moveFwd(-dt);
        }
        if (IsKeyDown(KEY_D)) {
            // EnableCursor();
            camera.moveRight(dt);
            // DisableCursor();
        }
        if (IsKeyDown(KEY_A)) {
            camera.moveRight(-dt);
        }
        if (IsKeyDown(KEY_SPACE)) {
            camera.position += camera.up * dt * camera.speed;
        }
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            camera.position -= camera.up * dt * camera.speed;
        }
        BeginDrawing();
        BeginMode3D(camera);
		ClearBackground(WHITE);

        for (const auto& chunk: chunks) {
            chunk->drawMesh();
        }

        EndMode3D();
        drawFPS();
		EndDrawing();
    }

    TextureLoader::unloadTextures();
 
}

int main() {
    InitWindow(SCWIDTH, SCHEIGHT, "Minecraft");
    std::filesystem::current_path("../");
	
    SetWindowFocused();

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