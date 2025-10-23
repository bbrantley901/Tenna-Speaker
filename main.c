/*
TODO: Add button support with callback functions 
      
*/

/* INCLUDES */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"

/* CONSTANTS */

/* Audio Data Constants */
#include "tenna_talking.h"

#define I2S_DATA_PIN    (19U)
#define I2S_BCLK_PIN    (20U)
#define I2S_LRCLK_PIN   (21U)

#define SAMPLE_RATE     (44100U)
#define BITS_PER_SAMPLE (16U)

/* Cast uint8_t array to int16_t */
static const int16_t *audio_samples = (const int16_t *)tenna_talking;

/* Sample Size is array length / 2 ; 2 bytes/sample*/
static const size_t audio_sample_count = tenna_talking_len / 2;

struct audio_file {
    const int16_t *samples;
    size_t sample_count;
};


/* PRIVATE VARIABLES */
static struct audio_file audio_file_list[] = {
    {
        .samples = (const int16_t *)tenna_talking,
        .sample_count = (sizeof(tenna_talking) / 2)
    },
};
static struct audio_file *current_audio_file; 
static audio_buffer_pool_t *ap = NULL;
static uint32_t volume = 32;

/* PRIVATE FUNCTION PROTOTYPES */
static audio_buffer_pool_t *init_audio(void);
static void play_audio(const int16_t *audio_samples, size_t sample_count);

/* PUBLIC FUNCTION IMPLEMENTATIONS */
int main()
{
    stdio_init_all();
    ap = init_audio();
    current_audio_file = &audio_file_list[0];
    while(1)
    {
        play_audio(current_audio_file->samples, current_audio_file->sample_count);
        sleep_ms(2000);
    }

#if 0
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
#endif 
    return 0;
}

static void play_audio(const int16_t *audio_samples, size_t sample_count)
{
    size_t pos = 0;
    audio_buffer_t *buffer = NULL;
    int16_t *samples = NULL;
    size_t count = 0;
    while (pos < sample_count) { // While there are samples left to play
        buffer = take_audio_buffer(ap, true); 
        samples = (int16_t *) buffer->buffer->bytes;
        count = buffer->max_sample_count;
        for (int i = 0; i < count; i++) { // Fill audio buffer with 256 bytes of samples
            int16_t s = audio_samples[pos++];
            samples[i] = s;
            samples[i+1] = s; 
            if (pos >= audio_sample_count) {
              buffer->sample_count = i; //Only process the samples we have
              break; // No more samples left; exit allocation of audio buffer
            }
        }
        buffer->sample_count = count;
        give_audio_buffer(ap, buffer); // Play audio buffer
    }
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