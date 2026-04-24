#ifndef MADGWICK_H
#define MADGWICK_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float q0, q1, q2, q3;
    float beta;
    float roll;
    float pitch;
    float yaw;
    bool initialised;
} Madgwick_t;

void Madgwick_Init(Madgwick_t *mw, float beta);
void Madgwick_UpdateMARG(Madgwick_t *mw,
                         float gx, float gy, float gz,
                         float ax, float ay, float az,
                         float mx, float my, float mz,
                         float dt);
void Madgwick_UpdateIMU(Madgwick_t *mw,
                        float gx, float gy, float gz,
                        float ax, float ay, float az,
                        float dt);
void Madgwick_GetEuler(Madgwick_t *mw);

#endif /* MADGWICK_H */
