/* INCLUDES */
#include <stdio.h>
#include "pico/stdlib.h"
#include "tenna_talking.h"

/* CONSTANTS */
#define I2S_DATA_PIN    (19U)
#define I2S_BCLK_PIN    (20U)
#define I2S_LRCLK_PIN   (21U)
#include "pico/audio_i2s.h"

#define SAMPLE_RATE     (44100U)
#define BITS_PER_SAMPLE (16U)

const int16_t *audio_samples = (const int16_t *)tenna_talking;
size_t audio_sample_count = tenna_talking_len / 2; //2 Bytes/Sample
/* PRIVATE VARIABLES */
static uint32_t volume = 32; // no gain to start

audio_i2s_config_t config = {
    .data_pin = I2S_DATA_PIN,
    .clock_pin_base = I2S_BCLK_PIN,
    .dma_channel = 0,
    .pio_sm = 0
};

/* PRIVATE FUNCTION PROTOTYPES */
static audio_buffer_pool_t *init_audio(void);

/* PUBLIC FUNCTION IMPLEMENTATIONS */
int main()
{
    stdio_init_all();
    audio_buffer_pool_t *ap = init_audio();
    size_t pos = 0;
    while (true)
    {
        audio_buffer_t *buffer = take_audio_buffer(ap, true);
        int16_t *samples = (int16_t *) buffer->buffer->bytes;
        size_t count = buffer->max_sample_count;
        for (uint i = 0; i < count; i++) {
            int16_t s = audio_samples[pos++];
            samples[i] = s;
            samples[i+1] = s;
            if (pos >= audio_sample_count) pos = 0;
        }
        buffer->sample_count = count;
        give_audio_buffer(ap, buffer);
    }
    return 0;
}

static audio_buffer_pool_t *init_audio(void)
{
    static audio_format_t audio_format = {
        .format = AUDIO_BUFFER_FORMAT_PCM_S16,
        .sample_freq = SAMPLE_RATE,
        .channel_count = 1
    };

    static struct audio_buffer_format producer_format = {
        .format = &audio_format,
        .sample_stride = 2
    };

    audio_buffer_pool_t *producer_pool= audio_new_producer_pool(&producer_format, 3, 256);
    bool __unused ok;
    const struct audio_format *output_format;
    audio_i2s_config_t config = {
        .data_pin = I2S_DATA_PIN,
        .clock_pin_base = I2S_BCLK_PIN,
        .dma_channel = 0,
        .pio_sm = 0
    };
    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("I2S setup failed");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);
    return producer_pool;
}