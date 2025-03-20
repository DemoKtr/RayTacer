#pragma once
#ifndef MATRIX3_H
#define MATRIX3_H

#include <cmath>
#include <cassert>
#include <iostream>
#include "Math/Vectors/vector3.h" 

namespace Math {
    struct Matrix3 {
        float m[3][3];  // Reprezentacja macierzy w porz¹dku wierszowym

        // Konstruktor domyœlny – tworzy macierz jednostkow¹
        Matrix3() {
            m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
            m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
            m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
        }

        // Konstruktor inicjalizuj¹cy macierz z 9 wartoœci (wierszami)
        Matrix3(float a00, float a01, float a02,
            float a10, float a11, float a12,
            float a20, float a21, float a22)
        {
            m[0][0] = a00; m[0][1] = a01; m[0][2] = a02;
            m[1][0] = a10; m[1][1] = a11; m[1][2] = a12;
            m[2][0] = a20; m[2][1] = a21; m[2][2] = a22;
        }

        // Konstruktor ustawiaj¹cy wszystkie elementy macierzy na tê sam¹ wartoœæ
        explicit Matrix3(float val) {
            m[0][0] = val; m[0][1] = val; m[0][2] = val;
            m[1][0] = val; m[1][1] = val; m[1][2] = val;
            m[2][0] = val; m[2][1] = val; m[2][2] = val;
        }

        // Operator dodawania macierzy: result = A + B
        inline Matrix3 operator+(const Matrix3& mat) const {
            return Matrix3(
                m[0][0] + mat.m[0][0], m[0][1] + mat.m[0][1], m[0][2] + mat.m[0][2],
                m[1][0] + mat.m[1][0], m[1][1] + mat.m[1][1], m[1][2] + mat.m[1][2],
                m[2][0] + mat.m[2][0], m[2][1] + mat.m[2][1], m[2][2] + mat.m[2][2]
            );
        }

        // Operator odejmowania macierzy: result = A - B
        inline Matrix3 operator-(const Matrix3& mat) const {
            return Matrix3(
                m[0][0] - mat.m[0][0], m[0][1] - mat.m[0][1], m[0][2] - mat.m[0][2],
                m[1][0] - mat.m[1][0], m[1][1] - mat.m[1][1], m[1][2] - mat.m[1][2],
                m[2][0] - mat.m[2][0], m[2][1] - mat.m[2][1], m[2][2] - mat.m[2][2]
            );
        }

        // Mno¿enie macierzy przez skalar: result = A * scalar
        inline Matrix3 operator*(float scalar) const {
            return Matrix3(
                m[0][0] * scalar, m[0][1] * scalar, m[0][2] * scalar,
                m[1][0] * scalar, m[1][1] * scalar, m[1][2] * scalar,
                m[2][0] * scalar, m[2][1] * scalar, m[2][2] * scalar
            );
        }

        // Dzielenie macierzy przez skalar: result = A / scalar
        inline Matrix3 operator/(float scalar) const {
            assert(scalar != 0.0f && "Division by zero");
            return *this * (1.0f / scalar);
        }

        // Mno¿enie macierzy przez macierz: result = A * B
        inline Matrix3 operator*(const Matrix3& mat) const {
            Matrix3 result(0.0f);  // Inicjalizujemy macierz zerow¹
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    result.m[i][j] = m[i][0] * mat.m[0][j] +
                        m[i][1] * mat.m[1][j] +
                        m[i][2] * mat.m[2][j];
                }
            }
            return result;
        }

        // Mno¿enie macierzy przez wektor (przyjmujemy, ¿e wektor jest kolumnowy)
        inline Vector3 operator*(const Vector3& v) const {
            return Vector3(
                m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
            );
        }

        // Transpozycja macierzy
        inline Matrix3 transpose() const {
            return Matrix3(
                m[0][0], m[1][0], m[2][0],
                m[0][1], m[1][1], m[2][1],
                m[0][2], m[1][2], m[2][2]
            );
        }

        // Obliczenie wyznacznika macierzy
        inline float determinant() const {
            return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
                + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        }

        // Odwracanie macierzy (inwersja)
        inline Matrix3 inverse() const {
            float det = determinant();
            assert(det != 0.0f && "Matrix is singular and cannot be inverted.");
            float invDet = 1.0f / det;
            return Matrix3(
                (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet,
                -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invDet,
                (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet,

                -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invDet,
                (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet,
                -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invDet,

                (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet,
                -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invDet,
                (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet
            );
        }
    };

    // Operator mno¿enia przez skalar z lewej strony: result = scalar * A
    inline Matrix3 operator*(float scalar, const Matrix3& mat) {
        return mat * scalar;
    }

    // Operator strumieniowy do wypisywania macierzy
    inline std::ostream& operator<<(std::ostream& os, const Matrix3& mat) {
        os << "Matrix3(\n";
        os << "  " << mat.m[0][0] << " " << mat.m[0][1] << " " << mat.m[0][2] << "\n";
        os << "  " << mat.m[1][0] << " " << mat.m[1][1] << " " << mat.m[1][2] << "\n";
        os << "  " << mat.m[2][0] << " " << mat.m[2][1] << " " << mat.m[2][2] << "\n";
        os << ")";
        return os;
    }
}

#endif // MATRIX3_H