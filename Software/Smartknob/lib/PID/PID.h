/*
 *  Title: PID Library

 *  Description: A library implementing a PID control system
 * 
 *  Author: Jordan (Perry)
 */

#pragma once
#include <array>

namespace SMARTKNOB
{
    enum class ErrorMode : unsigned char
    {
        LINEAR,
        ANGULAR,
    };

    enum class ProportionalMode : unsigned char
    {
        PROPORTIONAL_ON_ERROR,
        PROPORTIONAL_ON_MEASUREMENT,
    };

    enum class DerivativeMode : unsigned char
    {
        DERIVATIVE_ON_ERROR,
        DERIVATIVE_ON_ERROR_FILTERED,
        DERIVATIVE_ON_MEASUREMENT,
        DERIVATIVE_ON_MEASUREMENT_FILTERED,
    };

    class PID
    {
    public:
        float kP;
        float kI;
        float kD;
        float N; // 
        float setpoint;

        float error;

        float antiwindup;
        bool enableAntiwindup = false;

        float output;

        ErrorMode errorMode = ErrorMode::LINEAR;
        ProportionalMode proportionalMode = ProportionalMode::PROPORTIONAL_ON_ERROR;
        DerivativeMode derivativeMode = DerivativeMode::DERIVATIVE_ON_ERROR;

        PID()                                                       : kP(1), kI(0), kD(0), N(0), setpoint(0), antiwindup(0) {};
        PID(float p, float i, float d)                              : kP(p), kI(i), kD(d), N(1), setpoint(0), antiwindup(0) {};
        PID(float p, float i, float d, float n)                     : kP(p), kI(i), kD(d), N(n), setpoint(0), antiwindup(0) {};
        PID(float p, float i, float d, float n, float a)            : kP(p), kI(i), kD(d), N(1), setpoint(0), antiwindup(a), enableAntiwindup(true) {};

        float update(float pv, float dt);

        void reset() { setpoint = output = error = integrator = derivative = derivLast = 0; };
    private:
        const float _pi = 3.14159265358f;
        const float _3pi = 9.42477796076f;
        const float _2pi = 6.28318530717f;
        float integrator;
        float derivative;
        float derivLast;
    };
}