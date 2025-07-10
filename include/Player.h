#pragma once
#include "raylib.h"
#include "Vector3d.h"
#include <cmath>

class Player {
    public:
        Player(Vector3 position, float fovy = 90.0f, int perspective = CAMERA_PERSPECTIVE);
        operator Camera3D() const;
        void applyMatrix(const Matrix& matrix);

        void changePitch(float dx);
        void changeYaw(float dx);
        void update();

        void moveFwd(float dx);
        void moveRight(float dx);
        void moveUp(float dx);
        Vector3 getLocation() const;
        Vector3 getDirection() const;
        void setTargetBlock(Vector3 block);
        void drawHud() const;

        Vector2 getChunk() const;
    private:
        Vector3 m_position;
        Vector3 m_fwd;
        Vector3 m_up;
        Vector3 m_right;

        Vector3 m_targetBlock = {-INFINITY, -INFINITY, -INFINITY};

        float m_fovy;
        int m_perspective;

        float m_speed = 8.0f;
        float m_rotateSpeed = 0.07f; 
        float m_pitch = 0;
        float m_yaw = 0;



};