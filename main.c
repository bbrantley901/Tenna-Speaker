
/* INCLUDES */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include "hardware/gpio.h"

/* CONSTANTS */

/* Audio Data Constants */
#include "tenna_talking.h"
#include "its_tv_time.h"
#include "tenna_beep.h"
#include "crowd_cheer.h"
#include "crowd_laughter.h"
#include "crowd_gasp.h"

#define SAMPLE_RATE     (44100U)
#define BITS_PER_SAMPLE (16U)

/* GPIO Pins */

#define BUTTON_TENNA_TALKING_PIN  (1U) 
#define BUTTON_ITS_TV_TIME_PIN    (2U)  
#define BUTTON_TENNA_BEEP_PIN     (3U)  
#define BUTTON_CROWD_CHEER_PIN    (4U)  
#define BUTTON_CROWD_LAUGHTERPIN  (5U)  
#define BUTTON_CROWD_GASP_PIN     (6U)  
#define BUTTON_VOL_UP_PIN         (7U)
#define BUTTON_VOL_DOWN_PIN       (8U)
#define TOTAL_BUTTONS             (8U)
#define I2S_DATA_PIN              (19U)
#define I2S_BCLK_PIN              (20U)
#define I2S_LRCLK_PIN             (21U)

#define BUTTONS_GPIO_MASK ((1 << BUTTON_TENNA_TALKING_PIN) | \
                           (1 << BUTTON_ITS_TV_TIME_PIN)   | \
                           (1 << BUTTON_TENNA_BEEP_PIN)    | \
                           (1 << BUTTON_CROWD_CHEER_PIN)   | \
                           (1 << BUTTON_CROWD_LAUGHTERPIN) | \
                           (1 << BUTTON_CROWD_GASP_PIN)    | \
                           (1 << BUTTON_VOL_UP_PIN)        | \
                           (1 << BUTTON_VOL_DOWN_PIN))

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
    {
        .samples = (const int16_t *)its_tv_time,
        .sample_count = (sizeof(its_tv_time) / 2)
    },
    {
        .samples = (const int16_t *)tenna_beep,
        .sample_count = (sizeof(tenna_beep) / 2)
    },
    {
        .samples = (const int16_t *)crowd_cheer,
        .sample_count = (sizeof(crowd_cheer) / 2)
    },
    {
        .samples = (const int16_t *)crowd_laughter,
        .sample_count = (sizeof(crowd_laughter) / 2)
    },
    {
        .samples = (const int16_t *)crowd_gasp,
        .sample_count = (sizeof(crowd_gasp) / 2)
    }
};

static audio_buffer_pool_t *ap = NULL;
static volatile struct audio_file *current_audio_file; 
static volatile bool audio_queued = false;
static uint16_t volume = 128;

/* PRIVATE FUNCTION PROTOTYPES */

static audio_buffer_pool_t *init_audio(void);
static void play_audio(const int16_t *audio_samples, size_t sample_count);
static void change_audio_file_callback(uint gpio, uint32_t event_mask);

/* PUBLIC FUNCTION IMPLEMENTATIONS */

int main()
{
    stdio_init_all();

    /* Audio Setup */
    ap = init_audio();
    current_audio_file = &audio_file_list[1];

    /* Button Setup */
    gpio_init_mask(BUTTONS_GPIO_MASK);
    gpio_set_dir_in_masked(BUTTONS_GPIO_MASK);
    gpio_set_irq_enabled_with_callback(BUTTON_TENNA_TALKING_PIN, GPIO_IRQ_EDGE_FALL, true, change_audio_file_callback);
    for (int i = 1; i <= TOTAL_BUTTONS; i++) {
        gpio_pull_up(i);
        gpio_set_irq_enabled(i, GPIO_IRQ_EDGE_FALL, true);
    }

    /* Event Loop*/
    while(1)
    {   
        if (audio_queued) {
            play_audio(current_audio_file->samples, current_audio_file->sample_count); 
            audio_queued = false; 
        }

    }
    return 0;
}

/* Plays the audio located at the audio samples pointer */
static void play_audio(const int16_t *audio_samples, size_t sample_count)
{
    size_t pos = 0;
    audio_buffer_t *buffer = NULL;
    int16_t *samples = NULL;
    size_t count = 0;
    while (pos < sample_count) { // While there are samples left to play
        buffer = take_audio_buffer(ap, true); 
        samples = (int16_t *) buffer->buffer->bytes; // Our allocated memory for audio samples; Equal to the buffer sample count * sample_stride
        count = buffer->max_sample_count;
        for (int i = 0; i < count; i++) { // Fill audio buffer with 256 samples
          samples[i] = (volume * audio_samples[pos++]) >> 8U; // Volume control
          if (pos >= sample_count) {
            buffer->sample_count = i; //Only process the samples we have
            break; // No more samples left; exit allocation of audio buffer
          }
        }
        buffer->sample_count = count;
        give_audio_buffer(ap, buffer);
    }
}

/* Initializes an audio producer and returns a pointer to the variable */
static audio_buffer_pool_t *init_audio(void)
{
    static audio_format_t audio_format = {
        .format = AUDIO_BUFFER_FORMAT_PCM_S16,
        .sample_freq = SAMPLE_RATE,
        .channel_count = 1
    };

    static struct audio_buffer_format producer_format = {
        .format = &audio_format,
        .sample_stride = 2 // Basically the step size between samples in bytes; Our audio is 16bit PCM so 2 bytes
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

/* Set the current audio file based on which button was pressed. Inform main loop
   that audio has been queued. Playback is not done here to avoid playing audio in an 
   interrupt context */
static void change_audio_file_callback(uint gpio, uint32_t event_mask)
{
    audio_queued = true;
    switch (gpio) {
    case BUTTON_TENNA_TALKING_PIN:
        current_audio_file = &audio_file_list[0];
        break;
    case BUTTON_ITS_TV_TIME_PIN:
        current_audio_file = &audio_file_list[1];
        break;
    case BUTTON_TENNA_BEEP_PIN:
        current_audio_file = &audio_file_list[2];
        break;
    case BUTTON_CROWD_CHEER_PIN:
        current_audio_file = &audio_file_list[3];
        break;
    case BUTTON_CROWD_LAUGHTERPIN:
        current_audio_file = &audio_file_list[4];
        break;
    case BUTTON_CROWD_GASP_PIN:
        current_audio_file = &audio_file_list[5];
        break;
    case BUTTON_VOL_UP_PIN:
        if (volume <= 240) {
            volume += 16; 
        }
        audio_queued = false;
        break;
    case BUTTON_VOL_DOWN_PIN:
        if (volume >= 16) {
            volume -= 16; 
        }
        audio_queued = false;
        break;
    default:
        break;
    }
     
}