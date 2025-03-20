#pragma once
#ifndef VECTOR3_H
#define VECTOR3_H

#include <cmath>
#include <cassert>
#include <iostream>


namespace Math {
    struct Vector3 {
        float x, y, z;

        // Konstruktor domyœlny oraz inicjalizuj¹ce
        Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
        // Konstruktor ustawiaj¹cy wszystkie sk³adowe na tê sam¹ wartoœæ
        Vector3(float _x) : x(_x), y(_x), z(_x) {}

        // Operator dodawania dwóch wektorów
        inline Vector3 operator+(const Vector3& v) const {
            return Vector3(x + v.x, y + v.y, z + v.z);
        }

        // Operator odejmowania dwóch wektorów
        inline Vector3 operator-(const Vector3& v) const {
            return Vector3(x - v.x, y - v.y, z - v.z);
        }

        // Operator mno¿enia przez skalar (wektor * scalar)
        inline Vector3 operator*(float scalar) const {
            return Vector3(x * scalar, y * scalar, z * scalar);
        }

        // Operator dzielenia przez skalar
        inline Vector3 operator/(float scalar) const {
            assert(scalar != 0.0f && "Division by zero");
            return Vector3(x / scalar, y / scalar, z / scalar);
        }

        // Operator negacji (odwrócenie znaku wektora)
        inline Vector3 operator-() const {
            return Vector3(-x, -y, -z);
        }

        // Operatory przypisania z modyfikacj¹
        inline Vector3& operator+=(const Vector3& v) {
            x += v.x; y += v.y; z += v.z;
            return *this;
        }

        inline Vector3& operator-=(const Vector3& v) {
            x -= v.x; y -= v.y; z -= v.z;
            return *this;
        }

        inline Vector3& operator*=(float scalar) {
            x *= scalar; y *= scalar; z *= scalar;
            return *this;
        }

        inline Vector3& operator/=(float scalar) {
            assert(scalar != 0.0f && "Division by zero");
            x /= scalar; y /= scalar; z /= scalar;
            return *this;
        }

        // Obliczenie d³ugoœci (normy) wektora
        inline float length() const {
            return std::sqrt(x * x + y * y + z * z);
        }

        // Kwadrat d³ugoœci wektora (przydatne dla optymalizacji)
        inline float lengthSq() const {
            return x * x + y * y + z * z;
        }

        // Zwraca znormalizowany wektor (jednostkowy)
        inline Vector3 normalized() const {
            float len = length();
            if (len == 0.0f)
                return Vector3(0.0f, 0.0f, 0.0f);
            return *this / len;
        }

        // Iloczyn skalarny (dot product)
        inline float dot(const Vector3& v) const {
            return x * v.x + y * v.y + z * v.z;
        }

        // Iloczyn wektorowy (cross product)
        inline Vector3 cross(const Vector3& v) const {
            return Vector3(
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
            );
        }

        // Odleg³oœæ miêdzy dwoma wektorami
        inline float distance(const Vector3& v) const {
            return (*this - v).length();
        }

        // K¹t (w radianach) miêdzy wektorami: angle = acos((a·b)/(|a|*|b|))
        inline float angle(const Vector3& v) const {
            float lenProduct = length() * v.length();
            if (lenProduct == 0.0f)
                return 0.0f; // K¹t nieokreœlony, gdy któryœ z wektorów jest zerowy
            float cosTheta = dot(v) / lenProduct;
            // Ograniczenie wartoœci cosinusa do zakresu [-1, 1] by unikn¹æ b³êdów numerycznych
            if (cosTheta > 1.0f) cosTheta = 1.0f;
            if (cosTheta < -1.0f) cosTheta = -1.0f;
            return std::acos(cosTheta);
        }
    };

    // Operator mno¿enia przez skalar z lewej strony: scalar * vector
    inline Vector3 operator*(float scalar, const Vector3& v) {
        return v * scalar;
    }

    // Operator strumieniowy do wypisywania wektora
    inline std::ostream& operator<<(std::ostream& os, const Vector3& v) {
        os << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
}


#endif // VECTOR3_H
