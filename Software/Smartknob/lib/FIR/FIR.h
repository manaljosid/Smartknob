/*
 *  Title: FIR Library

 *  Description: FIR LPF or HPF filter of type 1 with Hamming window of variable length
 * 
 *  Author: Mani Magnusson
 */

#pragma once
#include <vector>
#include <deque>

class FIR {
public:
    FIR();

    /**
     * @brief FIR filter type
    */
    enum Type : uint8_t {
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
        Type type;
        uint length;
        float cutoff;
        float sample_rate;
    };
    
    void init(FIR_params* params);
    float run(float input);
private:
    const float _pi = 3.14159265358979323846f;
    FIR_params _params;
    std::vector<float> _coeffs;
    std::deque<float> _buffer;
    std::vector<float> calculate_coeffs(FIR_params* params);
};