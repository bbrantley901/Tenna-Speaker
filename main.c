#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include "tenna_talking.h"

#define I2S_DATA_PIN    (26U)
#define IS2_BCLK_PIN    (27U)
#define I2S_LRCLK_PIN   (28U)

#define SAMPLE_RATE     (22050U)
#define BITS_PER_SAMPLE (16U)

/* 
struct audio_format audio_format = {
    .format = AUDIO_BUFFER_FORMAT_PCM_S16,
    .sample_freq = SAMPLE_RATE,
    .channel_count = 1,
};

struct audio_buffer_pool *init_audio() {
    static audio_format_t producer_format = {
    .format = AUDIO_BUFFER_FORMAT_PCM_S16,
    .sample_freq = SAMPLE_RATE,
    .channel_count = 1
    };

    static struct audio_i2s_config config = 
    {
        .data_pin = I2S_DATA_PIN,
        .clock_pin_base = IS2_BCLK_PIN,
        .dma_channel = 0,
        .pio_sm = 0,
        .lrclk_pin = I2S_LRCLK_PIN,
    };

    struct audio_buffer_pool *ap = audio_new_producer_pool(&producer_format, 3, 256);
}
i2s_config_t config = {
    .data_pin = ,
    .clock_pin_base = ,
    .dma_channel = 0,
    .pio_sm = 0,
    .smple_freq = SAMPLE_RATE,
    .bits_per_sample = BITS_PER_SAMPLE,
    .format = I2S_DATA_FORMMAT_I2S
};
*/


int main()
{
    stdio_init_all();
}
