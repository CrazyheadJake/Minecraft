#include "raylib.h"

struct Vector3d {
    double x;
    double y;
    double z;

    Vector3d operator+(const Vector3d& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }
    Vector3d operator-(const Vector3d& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }
    Vector3d operator*(const Vector3d& other) const {
        return {x * other.x, y * other.y, z * other.z};
    }

    Vector3d operator*(double scalar) const {
        return {scalar * x, scalar * y, scalar * z};
    }
    Vector3d operator/(double scalar) const {
        return {scalar / x, scalar / y, scalar / z};
    }

    operator Vector3() const {
        return {(float)x, (float)y, (float)z};
    }
};