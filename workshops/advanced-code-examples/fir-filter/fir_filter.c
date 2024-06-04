
#include "fir_filter.h"

void fir_filter_init(fir_filter *filter, float *filter_coeff , float *buffer, const uint8_t taps){
    filter->out = 0.0f;
    filter->coef = filter_coeff;
    filter->buffer = buffer;
    filter->taps = taps;
    filter->index = 0;

    for(uint8_t n = 0; n < filter->taps; n++){
        filter->buffer[n] = 0.0f;
    }
}

float fir_filter_comp(fir_filter *filter, float input){
    filter->buffer[filter->index] = input;
    uint8_t getIndex = filter->index;
    filter->out = 0.0f;

    for (uint8_t n = 0; n < filter->taps; n++){
        filter->out = filter->out + (filter->buffer[getIndex]*filter->coef[n]);

        if(getIndex == 0){
            getIndex = filter->taps-1;
        }else{
            getIndex--;
        }
    }
    filter->index++;
    if( filter->index == filter->taps){
        filter->index = 0;
    }
    
    return filter->out;
}