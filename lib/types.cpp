#include <cmath>
#include <cstdlib>
#include <cstring>

#include "macs.hpp"

using namespace macs::types;


mat3 mat3::operator*(const mat3 &om) const
{
    float nd[9] = {
        d[0] * om.d[0] + d[3] * om.d[1] + d[6] * om.d[2],
        d[1] * om.d[0] + d[4] * om.d[1] + d[7] * om.d[2],
        d[2] * om.d[0] + d[5] * om.d[1] + d[8] * om.d[2],
        d[0] * om.d[3] + d[3] * om.d[4] + d[6] * om.d[5],
        d[1] * om.d[3] + d[4] * om.d[4] + d[7] * om.d[5],
        d[2] * om.d[3] + d[5] * om.d[4] + d[8] * om.d[5],
        d[0] * om.d[6] + d[3] * om.d[7] + d[6] * om.d[8],
        d[1] * om.d[6] + d[4] * om.d[7] + d[7] * om.d[8],
        d[2] * om.d[6] + d[5] * om.d[7] + d[8] * om.d[8]
    };

    return mat3(nd);
}

const mat3 &mat3::operator*=(const mat3 &om)
{
    float nd[9] = {
        d[0] * om.d[0] + d[3] * om.d[1] + d[6] * om.d[2],
        d[1] * om.d[0] + d[4] * om.d[1] + d[7] * om.d[2],
        d[2] * om.d[0] + d[5] * om.d[1] + d[8] * om.d[2],
        d[0] * om.d[3] + d[3] * om.d[4] + d[6] * om.d[5],
        d[1] * om.d[3] + d[4] * om.d[4] + d[7] * om.d[5],
        d[2] * om.d[3] + d[5] * om.d[4] + d[8] * om.d[5],
        d[0] * om.d[6] + d[3] * om.d[7] + d[6] * om.d[8],
        d[1] * om.d[6] + d[4] * om.d[7] + d[7] * om.d[8],
        d[2] * om.d[6] + d[5] * om.d[7] + d[8] * om.d[8]
    };

    memcpy(d, nd, sizeof(nd));

    return *this;
}

#define m2d(a1, a2, a3, a4) dt * (d[a1] * d[a2] - d[a3] * d[a4])

mat3 mat3::inv(void) const
{
    float dt = det();

    if (!dt)
        abort();

    dt = 1.f / dt;

    float i[9] = {
        m2d(4, 8, 5, 7), m2d(2, 7, 1, 8), m2d(1, 5, 2, 4),
        m2d(5, 6, 3, 8), m2d(0, 8, 2, 6), m2d(2, 3, 0, 5),
        m2d(3, 7, 4, 6), m2d(1, 6, 0, 7), m2d(0, 4, 1, 3)
    };

    return mat3(i);
}

void mat3::invert(void)
{
    float dt = det();

    if (!dt)
        abort();

    dt = 1.f / dt;

    float i[9] = {
        m2d(4, 8, 5, 7), m2d(2, 7, 1, 8), m2d(1, 5, 2, 4),
        m2d(5, 6, 3, 8), m2d(0, 8, 2, 6), m2d(2, 3, 0, 5),
        m2d(3, 7, 4, 6), m2d(1, 6, 0, 7), m2d(0, 4, 1, 3)
    };

    memcpy(d, i, sizeof(i));
}


mat4 mat4::operator*(const mat4 &om) const
{
    float nd[16] = {
        d[ 0] * om.d[ 0] + d[ 4] * om.d[ 1] + d[ 8] * om.d[ 2] + d[12] * om.d[ 3],
        d[ 1] * om.d[ 0] + d[ 5] * om.d[ 1] + d[ 9] * om.d[ 2] + d[13] * om.d[ 3],
        d[ 2] * om.d[ 0] + d[ 6] * om.d[ 1] + d[10] * om.d[ 2] + d[14] * om.d[ 3],
        d[ 3] * om.d[ 0] + d[ 7] * om.d[ 1] + d[11] * om.d[ 2] + d[15] * om.d[ 3],
        d[ 0] * om.d[ 4] + d[ 4] * om.d[ 5] + d[ 8] * om.d[ 6] + d[12] * om.d[ 7],
        d[ 1] * om.d[ 4] + d[ 5] * om.d[ 5] + d[ 9] * om.d[ 6] + d[13] * om.d[ 7],
        d[ 2] * om.d[ 4] + d[ 6] * om.d[ 5] + d[10] * om.d[ 6] + d[14] * om.d[ 7],
        d[ 3] * om.d[ 4] + d[ 7] * om.d[ 5] + d[11] * om.d[ 6] + d[15] * om.d[ 7],
        d[ 0] * om.d[ 8] + d[ 4] * om.d[ 9] + d[ 8] * om.d[10] + d[12] * om.d[11],
        d[ 1] * om.d[ 8] + d[ 5] * om.d[ 9] + d[ 9] * om.d[10] + d[13] * om.d[11],
        d[ 2] * om.d[ 8] + d[ 6] * om.d[ 9] + d[10] * om.d[10] + d[14] * om.d[11],
        d[ 3] * om.d[ 8] + d[ 7] * om.d[ 9] + d[11] * om.d[10] + d[15] * om.d[11],
        d[ 0] * om.d[12] + d[ 4] * om.d[13] + d[ 8] * om.d[14] + d[12] * om.d[15],
        d[ 1] * om.d[12] + d[ 5] * om.d[13] + d[ 9] * om.d[14] + d[13] * om.d[15],
        d[ 2] * om.d[12] + d[ 6] * om.d[13] + d[10] * om.d[14] + d[14] * om.d[15],
        d[ 3] * om.d[12] + d[ 7] * om.d[13] + d[11] * om.d[14] + d[15] * om.d[15]
    };

    return mat4(nd);
}

const mat4 &mat4::operator*=(const mat4 &om)
{
    float nd[16] = {
        d[ 0] * om.d[ 0] + d[ 4] * om.d[ 1] + d[ 8] * om.d[ 2] + d[12] * om.d[ 3],
        d[ 1] * om.d[ 0] + d[ 5] * om.d[ 1] + d[ 9] * om.d[ 2] + d[13] * om.d[ 3],
        d[ 2] * om.d[ 0] + d[ 6] * om.d[ 1] + d[10] * om.d[ 2] + d[14] * om.d[ 3],
        d[ 3] * om.d[ 0] + d[ 7] * om.d[ 1] + d[11] * om.d[ 2] + d[15] * om.d[ 3],
        d[ 0] * om.d[ 4] + d[ 4] * om.d[ 5] + d[ 8] * om.d[ 6] + d[12] * om.d[ 7],
        d[ 1] * om.d[ 4] + d[ 5] * om.d[ 5] + d[ 9] * om.d[ 6] + d[13] * om.d[ 7],
        d[ 2] * om.d[ 4] + d[ 6] * om.d[ 5] + d[10] * om.d[ 6] + d[14] * om.d[ 7],
        d[ 3] * om.d[ 4] + d[ 7] * om.d[ 5] + d[11] * om.d[ 6] + d[15] * om.d[ 7],
        d[ 0] * om.d[ 8] + d[ 4] * om.d[ 9] + d[ 8] * om.d[10] + d[12] * om.d[11],
        d[ 1] * om.d[ 8] + d[ 5] * om.d[ 9] + d[ 9] * om.d[10] + d[13] * om.d[11],
        d[ 2] * om.d[ 8] + d[ 6] * om.d[ 9] + d[10] * om.d[10] + d[14] * om.d[11],
        d[ 3] * om.d[ 8] + d[ 7] * om.d[ 9] + d[11] * om.d[10] + d[15] * om.d[11],
        d[ 0] * om.d[12] + d[ 4] * om.d[13] + d[ 8] * om.d[14] + d[12] * om.d[15],
        d[ 1] * om.d[12] + d[ 5] * om.d[13] + d[ 9] * om.d[14] + d[13] * om.d[15],
        d[ 2] * om.d[12] + d[ 6] * om.d[13] + d[10] * om.d[14] + d[14] * om.d[15],
        d[ 3] * om.d[12] + d[ 7] * om.d[13] + d[11] * om.d[14] + d[15] * om.d[15]
    };

    memcpy(d, nd, sizeof(nd));

    return *this;
}

#define te(a, b, c) d[a] * d[b] * d[c]
#define row(x, a1, a2, a3, a4, a5, a6, a7, a8, a9) d[x] * (te(a1, a2, a3) + te(a4, a5, a6) + te(a7, a8, a9))

float mat4::det(void) const
{
        return row( 0,   5, 10, 15,   9, 14,  7,  13,  6, 11) +
               row( 4,   1, 14, 11,   9,  2, 15,  13, 10,  3) +
               row( 8,   1,  6, 15,   5, 14,  3,  13,  2,  7) +
               row(12,   1, 10,  7,   5,  2, 11,   9,  6,  3) -
               row( 0,   5, 14, 11,   9,  6, 15,  13, 10,  7) -
               row( 4,   1, 10, 15,   9, 14,  3,  13,  2, 11) -
               row( 8,   1, 14,  7,   5,  2, 15,  13,  6,  3) -
               row(12,   1,  6, 11,   5, 10,  3,   9,  2,  7);
}

#define le(a1, a2, a3, a4, a5) d[a1] * (d[a2] * d[a3] - d[a4] * d[a5])

#define ele(x, a1, a2, a3, a4, a5, a6, a7, a8, a9, aA, aB, aC, aD, aE, aF) \
        x * (le(a1, a2, a3, a4, a5) + le(a6, a7, a8, a9, aA) + le(aB, aC, aD, aE, aF))

mat4 mat4::inv(void) const
{
    float dt = det();

    if (!dt)
        abort();

    dt = 1.f / dt;


    float nd[16] = {
        ele(dt,   5, 10, 15, 14, 11,    9, 14,  7,  6, 15,   13,  6, 11, 10,  7),
        ele(dt,   1, 14, 11, 10, 15,    9,  2, 15, 14,  3,   13, 10,  3,  2, 11),
        ele(dt,   1,  6, 15, 14,  7,    5, 14,  3,  2, 15,   13,  2,  7,  6,  3),
        ele(dt,   1, 10,  7,  6, 11,    5,  2, 11, 10,  3,    9,  6,  3,  2,  7),
        ele(dt,   4, 14, 11, 10, 15,    8,  6, 15, 14,  7,   12, 10,  7,  6, 11),
        ele(dt,   0, 10, 15, 14, 11,    8, 14,  3,  2, 15,   12,  2, 11, 10,  3),
        ele(dt,   0, 14,  7,  6, 15,    4,  2, 15, 14,  3,   12,  6,  3,  2,  7),
        ele(dt,   0,  6, 11, 10,  7,    4, 10,  3,  2, 11,    8,  2,  7,  6,  3),
        ele(dt,   4,  9, 15, 13, 11,    8, 13,  7,  5, 15,   12,  5, 11,  9,  7),
        ele(dt,   0, 13, 11,  9, 15,    8,  1, 15, 13,  3,   12,  9,  3,  1, 11),
        ele(dt,   0,  5, 15, 13,  7,    4, 13,  3,  1, 15,   12,  1,  7,  5,  3),
        ele(dt,   0,  9,  7,  5, 11,    4,  1, 11,  9,  3,    8,  5,  3,  1,  7),
        ele(dt,   4, 13, 10,  9, 14,    8,  5, 14, 13,  6,   12,  9,  6,  5, 10),
        ele(dt,   0,  9, 14, 13, 10,    8, 13,  2,  1, 14,   12,  1, 10,  9,  2),
        ele(dt,   0, 13,  6,  5, 14,    4,  1, 14, 13,  2,   12,  5,  2,  1,  6),
        ele(dt,   0,  5, 10,  9,  6,    4,  9,  2,  1, 10,    8,  1,  6,  5,  2)
    };

    return mat4(nd);
}

void mat4::invert(void)
{
    float dt = det();

    if (!dt)
        abort();

    dt = 1.f / dt;


    float nd[16] = {
        ele(dt,   5, 10, 15, 14, 11,    9, 14,  7,  6, 15,   13,  6, 11, 10,  7),
        ele(dt,   1, 14, 11, 10, 15,    9,  2, 15, 14,  3,   13, 10,  3,  2, 11),
        ele(dt,   1,  6, 15, 14,  7,    5, 14,  3,  2, 15,   13,  2,  7,  6,  3),
        ele(dt,   1, 10,  7,  6, 11,    5,  2, 11, 10,  3,    9,  6,  3,  2,  7),
        ele(dt,   4, 14, 11, 10, 15,    8,  6, 15, 14,  7,   12, 10,  7,  6, 11),
        ele(dt,   0, 10, 15, 14, 11,    8, 14,  3,  2, 15,   12,  2, 11, 10,  3),
        ele(dt,   0, 14,  7,  6, 15,    4,  2, 15, 14,  3,   12,  6,  3,  2,  7),
        ele(dt,   0,  6, 11, 10,  7,    4, 10,  3,  2, 11,    8,  2,  7,  6,  3),
        ele(dt,   4,  9, 15, 13, 11,    8, 13,  7,  5, 15,   12,  5, 11,  9,  7),
        ele(dt,   0, 13, 11,  9, 15,    8,  1, 15, 13,  3,   12,  9,  3,  1, 11),
        ele(dt,   0,  5, 15, 13,  7,    4, 13,  3,  1, 15,   12,  1,  7,  5,  3),
        ele(dt,   0,  9,  7,  5, 11,    4,  1, 11,  9,  3,    8,  5,  3,  1,  7),
        ele(dt,   4, 13, 10,  9, 14,    8,  5, 14, 13,  6,   12,  9,  6,  5, 10),
        ele(dt,   0,  9, 14, 13, 10,    8, 13,  2,  1, 14,   12,  1, 10,  9,  2),
        ele(dt,   0, 13,  6,  5, 14,    4,  1, 14, 13,  2,   12,  5,  2,  1,  6),
        ele(dt,   0,  5, 10,  9,  6,    4,  9,  2,  1, 10,    8,  1,  6,  5,  2)
    };

    memcpy(d, nd, sizeof(nd));
}


void mat4::rotate(float angle, vec3 axis)
{
    axis.norm();

    float s = sinf(angle);
    float c = cosf(angle);

    float omc = 1.f - c;


    float rot_mat[9] = {
        axis[0] * axis[0] * omc +           c,   axis[1] * axis[0] * omc + axis[2] * s,   axis[2] * axis[0] * omc - axis[1] * s,
        axis[0] * axis[1] * omc - axis[2] * s,   axis[1] * axis[1] * omc +           c,   axis[2] * axis[1] * omc + axis[0] * s,
        axis[0] * axis[2] * omc + axis[1] * s,   axis[1] * axis[2] * omc - axis[0] * s,   axis[2] * axis[2] * omc +           c
    };

    float nd[12] = {
        d[0] * rot_mat[0] + d[4] * rot_mat[1] + d[ 8] * rot_mat[2],
        d[1] * rot_mat[0] + d[5] * rot_mat[1] + d[ 9] * rot_mat[2],
        d[2] * rot_mat[0] + d[6] * rot_mat[1] + d[10] * rot_mat[2],
        d[3] * rot_mat[0] + d[7] * rot_mat[1] + d[11] * rot_mat[2],
        d[0] * rot_mat[3] + d[4] * rot_mat[4] + d[ 8] * rot_mat[5],
        d[1] * rot_mat[3] + d[5] * rot_mat[4] + d[ 9] * rot_mat[5],
        d[2] * rot_mat[3] + d[6] * rot_mat[4] + d[10] * rot_mat[5],
        d[3] * rot_mat[3] + d[7] * rot_mat[4] + d[11] * rot_mat[5],
        d[0] * rot_mat[6] + d[4] * rot_mat[7] + d[ 8] * rot_mat[8],
        d[1] * rot_mat[6] + d[5] * rot_mat[7] + d[ 9] * rot_mat[8],
        d[2] * rot_mat[6] + d[6] * rot_mat[7] + d[10] * rot_mat[8],
        d[3] * rot_mat[6] + d[7] * rot_mat[7] + d[11] * rot_mat[8]
    };

    memcpy(d, nd, sizeof(nd));
}


const vec4 &vec4::operator*=(const mat4 &m)
{ memcpy(d, (m * *this).d, sizeof(d)); return *this; }

const vec3 &vec3::operator*=(const mat3 &m)
{ memcpy(d, (m * *this).d, sizeof(d)); return *this; }
