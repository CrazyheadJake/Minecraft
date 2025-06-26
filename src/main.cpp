#include "raylib.h"
#include "raymath.h"
#include "MyCamera.h"
#include <iostream>
#include "BlockMesh.h"

const int SCWIDTH = 1280;
const int SCHEIGHT = 720;

void printMatrix(Matrix matrix) {
    std::cout << "[";
    std::cout << matrix.m0 << ", " << matrix.m4 << "," << matrix.m8 << "," << matrix.m12 << "," << std::endl;
    std::cout << matrix.m1 << ", " << matrix.m5 << "," << matrix.m9 << "," << matrix.m13 << "," << std::endl;
    std::cout << matrix.m2 << ", " << matrix.m6 << "," << matrix.m10 << "," << matrix.m14 << "," << std::endl;
    std::cout << matrix.m3 << ", " << matrix.m7 << "," << matrix.m11 << "," << matrix.m15 << "]" << std::endl;
}

void runGame() {
    MyCamera camera({0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, 90.0f, CAMERA_PERSPECTIVE);
    BlockMesh chunk;

    while (!WindowShouldClose()) {

        float dt = GetFrameTime();
        SetWindowFocused();

        Vector2 dXY = GetMouseDelta();
        SetMousePosition(SCWIDTH/2, SCHEIGHT/2);
        camera.changeYaw(dt * dXY.x * camera.rotateSpeed);
        camera.changePitch(dt * dXY.y * camera.rotateSpeed);
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

        chunk.drawMesh();

        EndMode3D();
		EndDrawing();
    }
 
}

int main() {
    InitWindow(SCWIDTH, SCHEIGHT, "Minecraft");
	
    DisableCursor();
    SetWindowFocused();
    ToggleFullscreen();

    runGame();

    CloseWindow();
    return 0;
}