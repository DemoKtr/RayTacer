#pragma once


#ifndef VECTOR4_H
#define VECTOR4_H

#include <cmath>
#include <cassert>
#include <iostream>
namespace Math {
    struct Vector4 {
        float x, y, z, w;

        // Konstruktor domyœlny – inicjalizuje wszystkie sk³adowe zerami
        Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

        // Konstruktor inicjalizuj¹cy podaj¹c cztery wartoœci
        Vector4(float _x, float _y, float _z, float _w)
            : x(_x), y(_y), z(_z), w(_w) {
        }

        // Konstruktor ustawiaj¹cy wszystkie sk³adowe na tê sam¹ wartoœæ
        Vector4(float _x)
            : x(_x), y(_x), z(_x), w(_x) {
        }

        // Operator dodawania: result = a + b
        inline Vector4 operator+(const Vector4& v) const {
            return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
        }

        // Operator odejmowania: result = a - b
        inline Vector4 operator-(const Vector4& v) const {
            return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
        }

        // Operator mno¿enia przez skalar: result = a * scalar
        inline Vector4 operator*(float scalar) const {
            return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
        }

        // Operator dzielenia przez skalar: result = a / scalar
        inline Vector4 operator/(float scalar) const {
            assert(scalar != 0.0f && "Division by zero");
            return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
        }

        // Operator negacji: result = -a
        inline Vector4 operator-() const {
            return Vector4(-x, -y, -z, -w);
        }

        // Operatory przypisania z modyfikacj¹
        inline Vector4& operator+=(const Vector4& v) {
            x += v.x; y += v.y; z += v.z; w += v.w;
            return *this;
        }

        inline Vector4& operator-=(const Vector4& v) {
            x -= v.x; y -= v.y; z -= v.z; w -= v.w;
            return *this;
        }

        inline Vector4& operator*=(float scalar) {
            x *= scalar; y *= scalar; z *= scalar; w *= scalar;
            return *this;
        }

        inline Vector4& operator/=(float scalar) {
            assert(scalar != 0.0f && "Division by zero");
            x /= scalar; y /= scalar; z /= scalar; w /= scalar;
            return *this;
        }

        // Oblicza d³ugoœæ (normê) wektora
        inline float length() const {
            return std::sqrt(x * x + y * y + z * z + w * w);
        }

        // Kwadrat d³ugoœci wektora – przydatny dla optymalizacji
        inline float lengthSq() const {
            return x * x + y * y + z * z + w * w;
        }

        // Zwraca znormalizowany wektor (wektor jednostkowy)
        inline Vector4 normalized() const {
            float len = length();
            if (len == 0.0f)
                return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
            return *this / len;
        }

        // Iloczyn skalarny (dot product)
        inline float dot(const Vector4& v) const {
            return x * v.x + y * v.y + z * v.z + w * v.w;
        }

        // Odleg³oœæ miêdzy wektorami
        inline float distance(const Vector4& v) const {
            return (*this - v).length();
        }

        // K¹t (w radianach) miêdzy wektorami:
        // angle = acos( (a·b) / (|a| * |b|) )
        inline float angle(const Vector4& v) const {
            float lenProduct = length() * v.length();
            if (lenProduct == 0.0f)
                return 0.0f; // K¹t nieokreœlony, gdy jeden z wektorów jest zerowy
            float cosTheta = dot(v) / lenProduct;
            // Ograniczamy wartoœæ do zakresu [-1, 1] by unikn¹æ b³êdów numerycznych
            if (cosTheta > 1.0f) cosTheta = 1.0f;
            if (cosTheta < -1.0f) cosTheta = -1.0f;
            return std::acos(cosTheta);
        }
    };

    // Operator mno¿enia przez skalar z lewej strony: scalar * vector
    inline Vector4 operator*(float scalar, const Vector4& v) {
        return v * scalar;
    }

    // Operator strumieniowy do wypisywania wektora
    inline std::ostream& operator<<(std::ostream& os, const Vector4& v) {
        os << "Vector4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }
}


#endif // VEC4_H
