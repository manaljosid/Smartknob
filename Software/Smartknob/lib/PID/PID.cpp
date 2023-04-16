/*
 *  Title: FIR Library

 *  Description: FIR LPF or HPF filter of type 1 with Hamming window of variable length
 * 
 *  Author: Jordan (Perry) and Mani Magnusson
 */

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <array>
#include "pico/stdlib.h"
#include "PID.h"

// PID update function
float SMARTKNOB::PID::update(float pv, float dt)
{
    // Calculate this timestep's error, based on setpoint and input
    error = setpoint - pv;

    // Convert 0-360 to -180-180 for angular errors
    if(errorMode == ErrorMode::ANGULAR)
    {
        error = fmodf(error + _3pi, _2pi) - _pi;
    }

    // Integrate and apply antiwindup clamp
    integrator += error * dt;
    if(enableAntiwindup)
    {
        if(integrator > antiwindup) // Upper bound
        {
            integrator = antiwindup;
        }
        else if(integrator < -antiwindup) // Lower bound
        {
            integrator = -antiwindup;
        }
    }

    // Calculate this timestep's derivative
    if(derivativeMode == DerivativeMode::DERIVATIVE_ON_ERROR)
    {
        derivative = (error - derivLast) / dt;
        derivLast = error;
    }
    else if(derivativeMode == DerivativeMode::DERIVATIVE_ON_ERROR_FILTERED)
    {
        derivative = (derivative + N * (error - derivLast)) / (1 + N * dt);
        derivLast = error;
    }
    else if(derivativeMode == DerivativeMode::DERIVATIVE_ON_MEASUREMENT)
    {
        derivative = (pv - derivLast) / dt;
        derivLast = pv;
    } 
    else if(derivativeMode == DerivativeMode::DERIVATIVE_ON_MEASUREMENT_FILTERED)
    {
        derivative = (derivative + N * (pv - derivLast)) / (1 + N * dt);
        derivLast = pv;
    } 

    // Sum the total controller output
    if(proportionalMode == ProportionalMode::PROPORTIONAL_ON_ERROR)
    {
        output = (kP * error) + (kI * integrator) + (kD * derivative);
    }
    else if(proportionalMode == ProportionalMode::PROPORTIONAL_ON_MEASUREMENT)
    {
        output = (kP * pv) + (kI * integrator) + (kD * derivative);
    }

    return output;
}