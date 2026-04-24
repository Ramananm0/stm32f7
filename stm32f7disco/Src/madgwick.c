#include "madgwick.h"
#include <math.h>

static float inv_sqrt(float x)
{
    float halfx = 0.5f * x;
    union {
        float f;
        uint32_t i;
    } conv = {x};

    conv.i = 0x5F3759DFu - (conv.i >> 1);
    conv.f = conv.f * (1.5f - halfx * conv.f * conv.f);
    return conv.f;
}

void Madgwick_Init(Madgwick_t *mw, float beta)
{
    mw->q0 = 1.0f;
    mw->q1 = 0.0f;
    mw->q2 = 0.0f;
    mw->q3 = 0.0f;
    mw->beta = beta;
    mw->roll = 0.0f;
    mw->pitch = 0.0f;
    mw->yaw = 0.0f;
    mw->initialised = false;
}

void Madgwick_UpdateMARG(Madgwick_t *mw,
                         float gx, float gy, float gz,
                         float ax, float ay, float az,
                         float mx, float my, float mz,
                         float dt)
{
    if (mx == 0.0f && my == 0.0f && mz == 0.0f) {
        Madgwick_UpdateIMU(mw, gx, gy, gz, ax, ay, az, dt);
        return;
    }

    /* This project only needs stable roll/pitch for the safety layer.
       Yaw correction is left to the magnetometer path when extended. */
    Madgwick_UpdateIMU(mw, gx, gy, gz, ax, ay, az, dt);
}

void Madgwick_UpdateIMU(Madgwick_t *mw,
                        float gx, float gy, float gz,
                        float ax, float ay, float az,
                        float dt)
{
    float q0 = mw->q0, q1 = mw->q1, q2 = mw->q2, q3 = mw->q3;
    float qDot0, qDot1, qDot2, qDot3;
    float rn;

    qDot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    qDot1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    qDot2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    qDot3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    if (!(ax == 0.0f && ay == 0.0f && az == 0.0f)) {
        float s0, s1, s2, s3;
        float _2q0 = 2.0f * q0, _2q1 = 2.0f * q1;
        float _2q2 = 2.0f * q2, _2q3 = 2.0f * q3;
        float _4q0 = 4.0f * q0, _4q1 = 4.0f * q1, _4q2 = 4.0f * q2;
        float _8q1 = 8.0f * q1, _8q2 = 8.0f * q2;
        float q0q0 = q0 * q0, q1q1 = q1 * q1;
        float q2q2 = q2 * q2, q3q3 = q3 * q3;

        rn = inv_sqrt(ax * ax + ay * ay + az * az);
        ax *= rn;
        ay *= rn;
        az *= rn;

        s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
        s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay -
             _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
        s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay -
             _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
        s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;

        rn = inv_sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
        s0 *= rn;
        s1 *= rn;
        s2 *= rn;
        s3 *= rn;

        qDot0 -= mw->beta * s0;
        qDot1 -= mw->beta * s1;
        qDot2 -= mw->beta * s2;
        qDot3 -= mw->beta * s3;
    }

    q0 += qDot0 * dt;
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;

    rn = inv_sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    mw->q0 = q0 * rn;
    mw->q1 = q1 * rn;
    mw->q2 = q2 * rn;
    mw->q3 = q3 * rn;
    mw->initialised = true;
    Madgwick_GetEuler(mw);
}

void Madgwick_GetEuler(Madgwick_t *mw)
{
    float q0 = mw->q0, q1 = mw->q1, q2 = mw->q2, q3 = mw->q3;

    mw->roll = atan2f(2.0f * (q0 * q1 + q2 * q3),
                      1.0f - 2.0f * (q1 * q1 + q2 * q2));
    mw->pitch = asinf(2.0f * (q0 * q2 - q3 * q1));
    mw->yaw = atan2f(2.0f * (q0 * q3 + q1 * q2),
                     1.0f - 2.0f * (q2 * q2 + q3 * q3));
}
