#pragma once
#include "raylib.h"
#include "raymath.h"
#include "Vector3d.h"
#include <cmath>
#include <mutex>
#include "VectorUtils.h"
#include "Block.h"

class Player {
    public:
        Player(Vector3 position, float fovy = 90.0f, int perspective = CAMERA_PERSPECTIVE);

        operator Camera3D() const { return {m_position, m_position + m_fwd, m_up, m_fovy, m_perspective}; }

        void applyMatrix(const Matrix& matrix);
        void changePitch(float dx);
        void changeYaw(float dx);
        void update();
        void moveFwd(float dx);
        void moveRight(float dx);
        void moveUp(float dx);
        Vector3 getLocation() const;
        Vector3 getDirection() const { return m_fwd; }
        float getFov() const { return m_fovy; }
        void setTargetBlock(Vector3 block) { m_targetBlock = Utils::floorVector(block, 1.0f); }
        void setHeldBlock(int i);
        Block getHeldBlock() { return m_selectedBlock; }
        void drawHud() const;
        Vector2 getChunk() const;

    private:
        mutable std::mutex m_positionMutex;
        Vector3 m_position;
        Vector3 m_fwd;
        Vector3 m_up;
        Vector3 m_right;

        Vector3 m_targetBlock = {-INFINITY, -INFINITY, -INFINITY};
        Block m_selectedBlock;

        float m_fovy;
        int m_perspective;

        float m_speed = 8.0f;
        float m_rotateSpeed = 0.07f; 
        float m_pitch = 0;
        float m_yaw = 0;
};