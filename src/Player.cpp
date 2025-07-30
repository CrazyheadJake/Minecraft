#include "Player.h"
#include "raymath.h"
#include "VectorUtils.h"
#include "BlockMesh.h"
#include <cmath>
#include "VectorUtils.h"
#include "Block.h"
#include "rlgl.h"

Player::Player(Vector3 position, float fovy, int perspective) : m_position(position), m_fovy(fovy), m_perspective(perspective) 
{
    m_fwd = {1, 0, 0};
    m_up = {0, 1, 0};
    m_right = {0, 0, 1};
    m_selectedBlock = Blocks::DIRT;
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
    Matrix yaw = MatrixRotate({0, 1, 0}, -Utils::toRadians(m_yaw));
    Matrix pitch = MatrixRotate({0, 0, 1}, -Utils::toRadians(m_pitch));
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

void Player::setHeldBlock(int i)
{
    if (i <= 2) return;
    m_selectedBlock = Block(i);
}

void Player::drawHud() const
{   
    int cursorSize = 3;
    DrawCircle(GetRenderWidth()/2.0f, GetRenderHeight()/2.0f, cursorSize, GRAY);
    // int hotbarSize = 64;
    // int hotbarElements = 10;
    // DrawRectangle(GetRenderWidth()/2.0f - hotbarSize * hotbarElements / 2.0f, GetRenderHeight() - hotbarSize, hotbarSize * hotbarElements, hotbarSize, LIGHTGRAY);
}

void Player::draw3DElements() const
{
    DrawCubeWires(m_targetBlock + Vector3{0.5f, 0.5f, 0.5f}, 1.002, 1.002, 1.002, BLACK);
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
