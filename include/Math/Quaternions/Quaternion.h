#pragma once
#ifndef QUATERNION_H
#define QUATERNION_H

#include <cmath>
#include <cassert>
#include <iostream>


namespace Math {
    struct Quaternion {
        float x, y, z, w;

        // Konstruktor domyślny – tworzy kwaternion jednostkowy (tożsamościowy)
        Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

        // Konstruktor inicjalizujący z czterema wartościami
        Quaternion(float _x, float _y, float _z, float _w)
            : x(_x), y(_y), z(_z), w(_w) {
        }

        // Konstruktor ustawiający wszystkie składowe na tę samą wartość (rzadko używany)
        explicit Quaternion(float val)
            : x(val), y(val), z(val), w(val) {
        }

        // Operator dodawania
        inline Quaternion operator+(const Quaternion& q) const {
            return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
        }

        // Operator odejmowania
        inline Quaternion operator-(const Quaternion& q) const {
            return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w);
        }

        // Operator mnożenia (iloczyn Hamiltona)
        inline Quaternion operator*(const Quaternion& q) const {
            return Quaternion(
                w * q.x + x * q.w + y * q.z - z * q.y,
                w * q.y - x * q.z + y * q.w + z * q.x,
                w * q.z + x * q.y - y * q.x + z * q.w,
                w * q.w - x * q.x - y * q.y - z * q.z
            );
        }

        // Operator mnożenia przez skalar
        inline Quaternion operator*(float scalar) const {
            return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
        }

        // Operator dzielenia przez skalar
        inline Quaternion operator/(float scalar) const {
            assert(scalar != 0.0f && "Division by zero");
            return Quaternion(x / scalar, y / scalar, z / scalar, w / scalar);
        }

        // Operator negacji
        inline Quaternion operator-() const {
            return Quaternion(-x, -y, -z, -w);
        }

        // Operatory przypisania z modyfikacją
        inline Quaternion& operator+=(const Quaternion& q) {
            x += q.x; y += q.y; z += q.z; w += q.w;
            return *this;
        }

        inline Quaternion& operator-=(const Quaternion& q) {
            x -= q.x; y -= q.y; z -= q.z; w -= q.w;
            return *this;
        }

        inline Quaternion& operator*=(const Quaternion& q) {
            *this = *this * q;
            return *this;
        }

        inline Quaternion& operator*=(float scalar) {
            x *= scalar; y *= scalar; z *= scalar; w *= scalar;
            return *this;
        }

        inline Quaternion& operator/=(float scalar) {
            assert(scalar != 0.0f && "Division by zero");
            x /= scalar; y /= scalar; z /= scalar; w /= scalar;
            return *this;
        }

        // Oblicza długość (normę) kwaternionu
        inline float length() const {
            return std::sqrt(x * x + y * y + z * z + w * w);
        }

        // Kwadrat długości – przydatne dla optymalizacji
        inline float lengthSq() const {
            return x * x + y * y + z * z + w * w;
        }

        // Zwraca znormalizowany kwaternion
        inline Quaternion normalized() const {
            float len = length();
            if (len == 0.0f)
                return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
            return *this / len;
        }

        // Sprzężenie kwaternionu (inwersja znaku części urojonej)
        inline Quaternion conjugate() const {
            return Quaternion(-x, -y, -z, w);
        }

        // Inwersja kwaternionu: q⁻¹ = q* / |q|²
        inline Quaternion inverse() const {
            float lenSq = lengthSq();
            assert(lenSq != 0.0f && "Cannot invert a zero-length quaternion");
            return conjugate() / lenSq;
        }

        // Iloczyn skalarny kwaternionów
        inline float dot(const Quaternion& q) const {
            return x * q.x + y * q.y + z * q.z + w * q.w;
        }
    };

    // Operator mnożenia przez skalar z lewej strony
    inline Quaternion operator*(float scalar, const Quaternion& q) {
        return q * scalar;
    }

    // Operator strumieniowy do wypisywania kwaternionu
    inline std::ostream& operator<<(std::ostream& os, const Quaternion& q) {
        os << "Quaternion(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
        return os;
    }
}


#endif // QUATERNION_H
