/*
 *  Title: FIR Library

 *  Description: FIR LPF or HPF filter of type 1 with Hamming window of variable length
 * 
 *  Author: Mani Magnusson
 */

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <array>
#include "pico/stdlib.h"
#include "FIR.h"

template <uint N> FIR<N>::FIR() {}

/******************************* PUBLIC METHODS *******************************/

/**
 * @brief Initialize the FIR library. Calculates coefficients and zero-initializes the buffer.
 * @param params Pointer to params struct containing FIR filter parameters
*/
template <uint N> void FIR<N>::init(FIR_params* params) {
    _params = *params;
    _coeffs = calculate_coeffs(params);
    for(uint i = 0; i < N; i++) {
        _buffer[i] = 0.0f;
    }
}

template <uint N> float FIR<N>::run(float input) {
    float output = 0.0f;
    _buffer[index] = input;
    index = index++;
    if(index == N) index = 0;
    uint sum_index = index;
    for(uint i = 0; i < N; i++) {
        if(sum_index > 0) {
            sum_index--;
        } else {
            sum_index = N-1;
        }
        output += _coeffs[i] * _buffer[sum_index];
    }
    return output;
}

/******************************* PRIVATE METHODS *******************************/

/**
 * @brief Calculate coefficients of a FIR filter. Uses a Hamming window and type 1 FIR filter (symmetric sequence of odd length)
 * @param params Pointer to params struct containing FIR filter parameters
 * @return std::array of type float containing the coefficients
*/
template <uint N> std::array<float, N> FIR<N>::calculate_coeffs(FIR_params* params) {
    float discrete_frequency = params->cutoff * 2.0f * _pi / params->sample_rate; // rad/s
    if(params->type == Type::HPF) discrete_frequency = _pi - discrete_frequency;
    uint length = N;
    std::array<float, N> coeffs;
    for(uint n = -((length-1)/2); n <= (length-1)/2; n++) {
        if(n == 0) {
            coeffs[n + (length-1)/2] = 1.0f;
        } else if(params->type == Type::HPF) {
            coeffs[n + (length-1)/2] = pow(-1.0f, float(n))*(sin(discrete_frequency * float(n)) / _pi * float(n)) * (0.54f * 0.46f * cos(2.0f * _pi * n / float(length)));
        } else {
            coeffs[n + (length-1)/2] = (sin(discrete_frequency * float(n)) / _pi * float(n)) * (0.54f * 0.46f * cos(2.0f * _pi * n / float(length)));
        }
    }
    return coeffs;
}