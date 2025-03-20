#pragma once
#ifndef MATRIX4_H
#define MATRIX4_H

#include <cmath>
#include <cassert>
#include <iostream>
#include "Math/Vectors/vector4.h"

namespace Math {
    struct Matrix4 {
        float m[4][4];  // Macierz 4x4 przechowywana w porz¹dku wierszowym

        // Konstruktor domyœlny – tworzy macierz jednostkow¹
        Matrix4() {
            m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
            m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
            m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
            m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
        }

        // Konstruktor inicjalizuj¹cy macierz przyjmuj¹c 16 wartoœci (wierszami)
        Matrix4(
            float a00, float a01, float a02, float a03,
            float a10, float a11, float a12, float a13,
            float a20, float a21, float a22, float a23,
            float a30, float a31, float a32, float a33
        ) {
            m[0][0] = a00; m[0][1] = a01; m[0][2] = a02; m[0][3] = a03;
            m[1][0] = a10; m[1][1] = a11; m[1][2] = a12; m[1][3] = a13;
            m[2][0] = a20; m[2][1] = a21; m[2][2] = a22; m[2][3] = a23;
            m[3][0] = a30; m[3][1] = a31; m[3][2] = a32; m[3][3] = a33;
        }

        // Konstruktor ustawiaj¹cy wszystkie elementy macierzy na dan¹ wartoœæ
        explicit Matrix4(float val) {
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    m[i][j] = val;
        }

        // Operator dodawania macierzy: result = A + B
        inline Matrix4 operator+(const Matrix4& mat) const {
            return Matrix4(
                m[0][0] + mat.m[0][0], m[0][1] + mat.m[0][1], m[0][2] + mat.m[0][2], m[0][3] + mat.m[0][3],
                m[1][0] + mat.m[1][0], m[1][1] + mat.m[1][1], m[1][2] + mat.m[1][2], m[1][3] + mat.m[1][3],
                m[2][0] + mat.m[2][0], m[2][1] + mat.m[2][1], m[2][2] + mat.m[2][2], m[2][3] + mat.m[2][3],
                m[3][0] + mat.m[3][0], m[3][1] + mat.m[3][1], m[3][2] + mat.m[3][2], m[3][3] + mat.m[3][3]
            );
        }

        // Operator odejmowania macierzy: result = A - B
        inline Matrix4 operator-(const Matrix4& mat) const {
            return Matrix4(
                m[0][0] - mat.m[0][0], m[0][1] - mat.m[0][1], m[0][2] - mat.m[0][2], m[0][3] - mat.m[0][3],
                m[1][0] - mat.m[1][0], m[1][1] - mat.m[1][1], m[1][2] - mat.m[1][2], m[1][3] - mat.m[1][3],
                m[2][0] - mat.m[2][0], m[2][1] - mat.m[2][1], m[2][2] - mat.m[2][2], m[2][3] - mat.m[2][3],
                m[3][0] - mat.m[3][0], m[3][1] - mat.m[3][1], m[3][2] - mat.m[3][2], m[3][3] - mat.m[3][3]
            );
        }

        // Mno¿enie macierzy przez skalar: result = A * scalar
        inline Matrix4 operator*(float scalar) const {
            return Matrix4(
                m[0][0] * scalar, m[0][1] * scalar, m[0][2] * scalar, m[0][3] * scalar,
                m[1][0] * scalar, m[1][1] * scalar, m[1][2] * scalar, m[1][3] * scalar,
                m[2][0] * scalar, m[2][1] * scalar, m[2][2] * scalar, m[2][3] * scalar,
                m[3][0] * scalar, m[3][1] * scalar, m[3][2] * scalar, m[3][3] * scalar
            );
        }

        // Dzielenie macierzy przez skalar: result = A / scalar
        inline Matrix4 operator/(float scalar) const {
            assert(scalar != 0.0f && "Division by zero");
            return (*this) * (1.0f / scalar);
        }

        // Mno¿enie macierzy przez macierz: result = A * B
        inline Matrix4 operator*(const Matrix4& mat) const {
            Matrix4 result(0.0f);
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    result.m[i][j] = m[i][0] * mat.m[0][j] +
                        m[i][1] * mat.m[1][j] +
                        m[i][2] * mat.m[2][j] +
                        m[i][3] * mat.m[3][j];
                }
            }
            return result;
        }

        // Mno¿enie macierzy przez wektor 4D (zak³adamy, ¿e wektor jest kolumnowy)
        inline Vector4 operator*(const Vector4& v) const {
            return Vector4(
                m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
                m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
                m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
                m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
            );
        }

        // Transpozycja macierzy
        inline Matrix4 transpose() const {
            return Matrix4(
                m[0][0], m[1][0], m[2][0], m[3][0],
                m[0][1], m[1][1], m[2][1], m[3][1],
                m[0][2], m[1][2], m[2][2], m[3][2],
                m[0][3], m[1][3], m[2][3], m[3][3]
            );
        }

        // Wyznacznik macierzy (determinant)
        inline float determinant() const {
            float det =
                m[0][0] * (
                    m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                    m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
                    m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])
                    )
                - m[0][1] * (
                    m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                    m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                    m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])
                    )
                + m[0][2] * (
                    m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
                    m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                    m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])
                    )
                - m[0][3] * (
                    m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
                    m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
                    m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])
                    );
            return det;
        }

        // Inwersja macierzy (odwrócenie macierzy)
        inline Matrix4 inverse() const {
            float det = determinant();
            assert(det != 0.0f && "Matrix is singular and cannot be inverted.");
            float invDet = 1.0f / det;

            Matrix4 inv;

            inv.m[0][0] = invDet * (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
            inv.m[0][1] = -invDet * (m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[0][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[0][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
            inv.m[0][2] = invDet * (m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) - m[0][2] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) + m[0][3] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]));
            inv.m[0][3] = -invDet * (m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) - m[0][2] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) + m[0][3] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]));

            inv.m[1][0] = -invDet * (m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]));
            inv.m[1][1] = invDet * (m[0][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - m[0][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[0][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]));
            inv.m[1][2] = -invDet * (m[0][0] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) - m[0][2] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) + m[0][3] * (m[1][0] * m[3][2] - m[1][2] * m[3][0]));
            inv.m[1][3] = invDet * (m[0][0] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) - m[0][2] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) + m[0][3] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]));

            inv.m[2][0] = invDet * (m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) - m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
            inv.m[2][1] = -invDet * (m[0][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) - m[0][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) + m[0][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
            inv.m[2][2] = invDet * (m[0][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) - m[0][1] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) + m[0][3] * (m[1][0] * m[3][1] - m[1][1] * m[3][0]));
            inv.m[2][3] = -invDet * (m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) - m[0][1] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) + m[0][3] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]));

            inv.m[3][0] = -invDet * (m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) - m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) + m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
            inv.m[3][1] = invDet * (m[0][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) - m[0][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) + m[0][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
            inv.m[3][2] = -invDet * (m[0][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) - m[0][1] * (m[1][0] * m[3][2] - m[1][2] * m[3][0]) + m[0][2] * (m[1][0] * m[3][1] - m[1][1] * m[3][0]));
            inv.m[3][3] = invDet * (m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]));

            return inv;
        }
    };

    // Operator mno¿enia przez skalar z lewej strony: result = scalar * Matrix4
    inline Matrix4 operator*(float scalar, const Matrix4& mat) {
        return mat * scalar;
    }

    // Operator strumieniowy do wypisywania macierzy 4x4
    inline std::ostream& operator<<(std::ostream& os, const Matrix4& mat) {
        os << "Matrix4(\n";
        os << "  " << mat.m[0][0] << " " << mat.m[0][1] << " " << mat.m[0][2] << " " << mat.m[0][3] << "\n";
        os << "  " << mat.m[1][0] << " " << mat.m[1][1] << " " << mat.m[1][2] << " " << mat.m[1][3] << "\n";
        os << "  " << mat.m[2][0] << " " << mat.m[2][1] << " " << mat.m[2][2] << " " << mat.m[2][3] << "\n";
        os << "  " << mat.m[3][0] << " " << mat.m[3][1] << " " << mat.m[3][2] << " " << mat.m[3][3] << "\n";
        os << ")";
        return os;
    }
}



#endif // MATRIX4_H
