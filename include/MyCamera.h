#include "raylib.h"
#include "Vector3d.h"

struct MyCamera {
    Vector3 position;
    Vector3 fwd;
    Vector3 up;
    Vector3 right;

    float fovy;
    int perspective;

    float speed = 5.0f;
    float rotateSpeed = 0.07f; 
    float pitch = 0;
    float yaw = 0;

    operator Camera3D() const;

    void applyMatrix(const Matrix& matrix);

    void changePitch(float dx);
    void changeYaw(float dx);
    void update();

    void moveFwd(float dx);
    void moveRight(float dx);
};