#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2s.h"


i2s_config_t config = {
    .data_pin = ,
    .clock_pin_base = ,
    .dma_channel = 0,
    .pio_sm = 0,
    .smple_freq = SAMPLE_RATE,
    .bits_per_sample = BITS_PER_SAMPLE,
    .format = I2S_DATA_FORMMAT_I2S
};

static void play_sample();

int main()
{
    stdio_init_all();
    i2s_init(&config);
    
}
