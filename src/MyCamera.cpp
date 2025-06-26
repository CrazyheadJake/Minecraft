#include "MyCamera.h"
#include "raymath.h"

float toRadians(float angle) {
    return angle / 180.0 * M_PI;
}

MyCamera::operator Camera3D() const
{
    return {position, position + fwd, up, fovy, perspective};
}

void MyCamera::applyMatrix(const Matrix &matrix)
{
    fwd = Vector3Transform(fwd, matrix);
    // up = Vector3Transform(up, matrix);
    right = Vector3Transform(right, matrix);
}

void MyCamera::changePitch(float dx)
{
    this->pitch += dx;
    if (this->pitch >= 90)
        this->pitch = 89.99;
    else if (this->pitch <= -90)
        this->pitch = -89.99;
}

void MyCamera::changeYaw(float dx)
{
    this->yaw += dx;
    if (this->yaw > 360)
        this->yaw -= 360;
}

void MyCamera::update()
{
    Matrix yaw = MatrixRotate({0, 1, 0}, -toRadians(this->yaw));
    Matrix pitch = MatrixRotate({0, 0, 1}, -toRadians(this->pitch));
    // camera.fwd = Vector3RotateByAxisAngle(camera.fwd, camera.up, -dt * dXY.x);
    this->fwd = {1, 0, 0};
    this->right = {0, 0, 1};

    this->applyMatrix(pitch);
    this->applyMatrix(yaw);
}

void MyCamera::moveFwd(float dx)
{
    Vector3 flatFwd = this->fwd;
    flatFwd.y = 0;
    flatFwd = Vector3Normalize(flatFwd);
    this->position += flatFwd * dx * this->speed;
}

void MyCamera::moveRight(float dx)
{
    Vector3 flatRight = this->right;
    flatRight.y = 0;
    flatRight = Vector3Normalize(flatRight);
    this->position += flatRight * dx * this->speed;
}