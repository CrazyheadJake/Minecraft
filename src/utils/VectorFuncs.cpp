#include "VectorUtils.h"
#include <cmath>
#include <functional>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Utils {
    float toRadians(float angle) {
        return angle / 180.0 * M_PI;
    }
    Vector2 floorVector(const Vector2& v, int precision) {
        return {floorf(v.x / precision) * precision, floorf(v.y / precision) * precision};
    }
    Vector3 floorVector(const Vector3& v, int precision) {
        return {floorf(v.x / precision) * precision, floorf(v.y / precision) * precision, floorf(v.z / precision) * precision};
    }
    Vector2 roundVector(const Vector2& v, int precision) {
        return {roundf(v.x / precision) * precision, roundf(v.y / precision) * precision};
    }
    Vector3 roundVector(const Vector3& v, int precision) {
        return {roundf(v.x / precision) * precision, roundf(v.y / precision) * precision, roundf(v.z / precision) * precision};
    }
};