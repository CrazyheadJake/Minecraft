#include "Player.h"
#include "raymath.h"
#include "VectorUtils.h"
#include "BlockMesh.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float toRadians(float angle) {
    return angle / 180.0 * M_PI;
}

Player::Player(Vector3 position, float fovy, int perspective) : m_position(position), m_fovy(fovy), m_perspective(perspective) 
{
    m_fwd = {1, 0, 0};
    m_up = {0, 1, 0};
    m_right = {0, 0, 1};
}

void Player::applyMatrix(const Matrix &matrix)
{
    m_fwd = Vector3Transform(m_fwd, matrix);
    // m_up = Vector3Transform(m_up, matrix);
    m_right = Vector3Transform(m_right, matrix);
}

void Player::changePitch(float dx)
{
    m_pitch += dx * m_rotateSpeed;
    if (m_pitch >= 90)
        m_pitch = 89.99;
    else if (m_pitch <= -90)
        m_pitch = -89.99;
}

void Player::changeYaw(float dx)
{
    m_yaw += dx * m_rotateSpeed;
    if (m_yaw > 360)
        m_yaw -= 360;
}

void Player::update()
{
    Matrix yaw = MatrixRotate({0, 1, 0}, -toRadians(m_yaw));
    Matrix pitch = MatrixRotate({0, 0, 1}, -toRadians(m_pitch));
    // camera.fwd = Vector3RotateByAxisAngle(camera.fwd, camera.up, -dt * dXY.x);
    m_fwd = {1, 0, 0};
    m_right = {0, 0, 1};

    applyMatrix(pitch);
    applyMatrix(yaw);
}

void Player::moveFwd(float dx)
{
    std::lock_guard<std::mutex> lock(m_positionMutex);
    Vector3 flatFwd = m_fwd;
    flatFwd.y = 0;
    flatFwd = Vector3Normalize(flatFwd);
    m_position += flatFwd * dx * m_speed;
}

void Player::moveRight(float dx)
{
    std::lock_guard<std::mutex> lock(m_positionMutex);
    Vector3 flatRight = m_right;
    flatRight.y = 0;
    flatRight = Vector3Normalize(flatRight);
    m_position += flatRight * dx * m_speed;
}

Vector3 Player::getLocation() const
{
    std::lock_guard<std::mutex> lock(m_positionMutex);
    return m_position;
}

void Player::drawHud() const
{   
    BeginMode3D(*this);
    DrawCubeWires(m_targetBlock + Vector3{0.5f, 0.5f, 0.5f}, 1, 1, 1, BLACK);
    EndMode3D();
    int cursorSize = 5;
    DrawRectangle(GetScreenWidth()/2.0f - cursorSize/2, GetScreenHeight()/2.0f - cursorSize/2, cursorSize, cursorSize, RED);
}

void Player::moveUp(float dt)
{
    std::lock_guard<std::mutex> lock(m_positionMutex);
    m_position += m_up * dt * m_speed;
}

Vector2 Player::getChunk() const
{
    std::lock_guard<std::mutex> lock(m_positionMutex);
    return Utils::floorVector(Vector2{m_position.x, m_position.z}, BlockMesh::LENGTH) / BlockMesh::LENGTH;
}
