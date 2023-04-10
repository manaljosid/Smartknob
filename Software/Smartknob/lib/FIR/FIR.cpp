/*
 *  Title: FOC Library

 *  Description: Calculates BLDC phase voltages based on motor angles and desired torques/speeds
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <deque>
#include "pico/stdlib.h"
#include "FIR.h"

template <typename T> T constrain(T amt, T low, T high) {
    if(amt < low) return low;
    if(amt > high) return high;
    return amt;
}

FIR::FIR() {
    //
}

/******************************* PUBLIC METHODS *******************************/

/**
 * @brief Initialize the FIR library. Calculates coefficients and zero-initializes the buffer.
 * @param params Pointer to params struct containing FIR filter parameters
*/
void FIR::init(FIR_params* params) {
    _params = *params;
    _coeffs = calculate_coeffs(params);
    for(uint i = 0; i < params->length; i++) {
        _buffer.push_front(0.0f);
    }
}

float FIR::run(float input) {
    _buffer.push_front(input);
    _buffer.pop_back();
    float output = 0.0f;
    for(uint i = 0; i < _params.length; i++) {
        output += _coeffs[i] * _buffer[i];
    }
    return output;
}

/******************************* PRIVATE METHODS *******************************/

/**
 * @brief Calculate coefficients of a FIR filter. Uses a Hamming window and type 1 FIR filter (symmetric sequence of odd length)
 * @param params Pointer to params struct containing FIR filter parameters
 * @return std::vector of type float containing the coefficients
*/
std::vector<float> FIR::calculate_coeffs(FIR_params* params) {
    float discrete_frequency = params->cutoff * 2.0f * _pi / params->sample_rate; // rad/s
    if(params->type == Type::HPF) discrete_frequency = _pi - discrete_frequency;
    uint length = params->length;
    if(length % 2 == 0) length++; // If length is even number make it odd by adding 1
    std::vector<float> coeffs;
    for(uint n = -((length-1)/2); n <= (length-1)/2; n++) {
        if(n == 0) {
            coeffs.push_back(1.0f);
        } else if(params->type == Type::HPF) {
            coeffs.push_back(pow(-1.0f, float(n))*(sin(discrete_frequency * float(n)) / _pi * float(n)) * (0.54f * 0.46f * cos(2.0f * _pi * n / float(length))));
        } else {
            coeffs.push_back((sin(discrete_frequency * float(n)) / _pi * float(n)) * (0.54f * 0.46f * cos(2.0f * _pi * n / float(length))));
        }
    }
    return coeffs;
}