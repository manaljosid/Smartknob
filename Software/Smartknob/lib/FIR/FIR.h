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
    FIR_type type;
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
    FIR();
    
    void init(FIR_params* params);
    float run(float input);
private:
    const float _pi = 3.14159265358979323846f;
    FIR_params _params;
    uint index = N-1;
    std::array<float, N> _coeffs;
    std::array<float, N> _buffer;
    std::array<float, N> calculate_coeffs(FIR_params* params);
};