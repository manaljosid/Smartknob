/*
 *  Title: FOC Library

 *  Description: Calculates BLDC phase voltages based on motor angles and desired torques/speeds
 * 
 *  Author: Mani Magnusson
 */

#pragma once
#include <vector>
/* TODO:
 - Add everything
*/

class FIR {
public:
    FIR();
    void init(void);

    bool update(void);
private:
};