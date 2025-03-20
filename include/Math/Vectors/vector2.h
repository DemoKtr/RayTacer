#pragma once
#ifndef VECTOR2_H
#define VECTOR2_H
#include <cmath>
#include <cassert>
#include <iostream>
namespace Math {




    struct Vector2 {
        float x, y;


        Vector2() : x(0.0f), y(0.0f) {}
        Vector2(float _x, float _y) : x(_x), y(_y) {}
        Vector2(float _x) : x(_x), y(_x) {}

        inline Vector2 operator+(const Vector2& v) const {
            return Vector2(x + v.x, y + v.y);
        }

        inline Vector2 operator-(const Vector2& v) const {
            return Vector2(x - v.x, y - v.y);
        }

        inline Vector2 operator*(float scalar) const {
            return Vector2(x * scalar, y * scalar);
        }

        inline Vector2 operator/(float scalar) const {
            assert(scalar != 0.0f && "Division by zero");
            return Vector2(x / scalar, y / scalar);
        }

        inline Vector2& operator+=(const Vector2& v) {
            x += v.x; y += v.y;
            return *this;
        }

        inline Vector2& operator-=(const Vector2& v) {
            x -= v.x; y -= v.y;
            return *this;
        }

        inline Vector2& operator*=(float scalar) {
            x *= scalar; y *= scalar;
            return *this;
        }

        inline Vector2& operator/=(float scalar) {
            assert(scalar != 0.0f && "Division by zero");
            x /= scalar; y /= scalar;
            return *this;
        }

        inline Vector2 operator-() const {
            return Vector2(-x, -y);
        }


        inline float length() const {
            return std::sqrt(x * x + y * y);
        }


        inline float lengthSq() const {
            return x * x + y * y;
        }

        inline Vector2 normalized() const {
            float len = length();
            if (len == 0.0f)
                return Vector2(0.0f, 0.0f);
            return (*this) / len;
        }

        inline float dot(const Vector2& v) const {
            return x * v.x + y * v.y;
        }

        inline float cross(const Vector2& v) const {
            return x * v.y - y * v.x;
        }

        inline float distance(const Vector2& v) const {
            return ((*this) - v).length();
        }

        // K¹t (w radianach) miêdzy dwoma wektorami
        inline float angle(const Vector2& v) const {
            float lenProduct = length() * v.length();
            if (lenProduct == 0.0f)
                return 0.0f; // K¹t nieokreœlony, gdy któryœ z wektorów jest zerowy
            float cos = dot(v) / lenProduct;
            // Ograniczenie wartoœci cosinusa do zakresu [-1, 1] by unikn¹æ b³êdów numerycznych
            if (cos > 1.0f) cos = 1.0f;
            if (cos < -1.0f) cos = -1.0f;
            return std::acos(cos);
        }
        // K¹t (w stopniach) miêdzy dwoma wektorami
        inline float angleDegrees(const Vector2& v) const {
            float lenProduct = length() * v.length();
            if (lenProduct == 0.0f)
                return 0.0f; // K¹t nieokreœlony, gdy któryœ z wektorów jest zerowy
            float cosAngle = dot(v) / lenProduct;
            // Ograniczenie wartoœci cosinusa do zakresu [-1, 1] by unikn¹æ b³êdów numerycznych
            if (cosAngle > 1.0f) cosAngle = 1.0f;
            if (cosAngle < -1.0f) cosAngle = -1.0f;
            float radians = std::acos(cosAngle);
            return radians * (180.0f / 3.14159265358979323846f);
        }
        // Obrót wektora o zadany k¹t (w radianach)
        inline Vector2 rotated(float angleRadians) const {
            float cosA = std::cos(angleRadians);
            float sinA = std::sin(angleRadians);
            return Vector2(x * cosA - y * sinA, x * sinA + y * cosA);
        }
    };

    // Operator mno¿enia przez skalara z lewej strony
    inline Vector2 operator*(float scalar, const Vector2& v) {
        return v * scalar;
    }

    // Opcjonalnie: operator strumieniowy do wypisywania wektora
    inline std::ostream& operator<<(std::ostream& os, const Vector2& v) {
        os << "Vector2(" << v.x << ", " << v.y << ")";
        return os;
    }



}
#endif // VECTOR2_H