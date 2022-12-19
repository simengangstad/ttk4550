#include "linalg.h"

namespace linalg {
    float determinant(const Mat<4>& source) {
        return source(0, 0) * source(1, 1) - source(0, 1) * source(1, 0);
    }

    // ----------------------------- Vector ----------------------------------

    Vec2::Vec2() : x(0), y(0) {}

    Vec2::Vec2(float xs, float ys) : x(xs), y(ys) {}

    float Vec2::norm() { return sqrt(x * x + y * y); }

    Vec2 Vec2::operator+(const Vec2& vector) const {
        Vec2 result(x + vector.x, y + vector.y);
        return result;
    }

    Vec2 Vec2::operator-(const Vec2& vector) const {
        Vec2 result(x - vector.x, y - vector.y);
        return result;
    }

    Vec2 operator*(const float alpha, const Vec2& vector) {
        Vec2 result(alpha * vector.x, alpha * vector.y);
        return result;
    }

}
