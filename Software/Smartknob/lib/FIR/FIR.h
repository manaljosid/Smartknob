/*
 *  Title: FIR Library

 *  Description: FIR LPF or HPF filter of type 1 with Hamming window of variable length
 * 
 *  Author: Mani Magnusson
 */

#pragma once
#include <array>

/**
 * @brief FIR filter type
*/
enum FIR_type : uint8_t {
    LPF = 0,
    HPF = 1
};

/**
 * @brief Struct containing the parameters for a FIR filter instance
 * @param type FIR filter type LPF or HPF (see FIR::Type)
 * @param length Length of the filter
 * @param cutoff Filter cutoff frequency in Hz
 * @param sample_rate Filter sample rate in Hz
*/
struct FIR_params {
    FIR_type type = FIR_type::LPF;
    float cutoff;
    float sample_rate;
};

/**
 * @brief FIR class
 * @param N Number of taps of the FIR filter - must be an odd number
*/
template <uint N> class FIR {
    static_assert((N % 2) != 0, "N is not an odd number");
public:
    FIR() {};
    
    /**
     * @brief Initialize the FIR library. Calculates coefficients and zero-initializes the buffer.
     * @param params Pointer to params struct containing FIR filter parameters
    */
    void init(FIR_params* params) {
        _params = *params;
        calculate_coeffs(params);
        for(uint i = 0; i < N; i++) {
            _buffer[i] = 0.0f;
        }
    };

    float run(float input) {
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
    };

    void dump_coeffs(void) {
        printf("N: %d\n\n", N);
        for(uint i = 0; i < N; i++) {
            printf("%f ", _coeffs[i]);
        }
        printf("\n");
    };
    void dump_buffer(void) {
        for(uint i = 0; i < N; i++) {
            printf("%f ", _buffer[i]);
        }
        printf("\n");
    };
private:
    const float _pi = 3.14159265358979323846f;
    FIR_params _params;
    uint index = N-1;
    std::array<float, N> _coeffs;
    std::array<float, N> _buffer;

    /**
     * @brief Calculate coefficients of a FIR filter. Uses a Hamming window and type 1 FIR filter (symmetric sequence of odd length)
     * @param params Pointer to params struct containing FIR filter parameters
     * @return std::array of type float containing the coefficients
    */
    void calculate_coeffs(FIR_params* params) {
        float discrete_frequency = params->cutoff * 2.0f * _pi / params->sample_rate; // rad/s
        if(params->type == FIR_type::HPF) discrete_frequency = _pi - discrete_frequency;
        uint length = N;
        std::array<float, N> coeffs;
        for(int n = (int)(-((length-1)/2)); n <= (int)((length-1)/2); n++) {
            if(n == 0) {
                coeffs[(int)(n + (length-1)/2)] = 1.0f;
            } else if(params->type == FIR_type::HPF) {
                coeffs[(int)(n + (length-1)/2)] = pow(-1.0f, (float)(n))*(sin(discrete_frequency * (float)(n)) / _pi * (float)(n)) * (0.54f * 0.46f * cos(2.0f * _pi * n / (float)(length)));
            } else {
                coeffs[(int)(n + (length-1)/2)] = (sin(discrete_frequency * (float)(n)) / _pi * (float)(n)) * (0.54f + 0.46f * cos(2.0f * _pi * n / (float)(length)));
            }
        }
        for(uint i = 0; i < N; i++) {
            _coeffs[i] = coeffs[i];
        }
    };
};