#ifndef FIR_FILTER_H
#define FIR_FILTER_H
#include <stdint.h>
#include "stm32f091xc.h"

typedef struct {
    float out;
    float *coef;
    float *buffer;
    uint8_t taps;
    uint8_t index;
    uint8_t filter_reg;
    /* data */
}fir_filter;



void fir_filter_init(fir_filter *filter, float *coeff , float *buffer, const uint8_t order);
float fir_filter_comp(fir_filter *filter, float input);

#endif // FIR_FILTER_H