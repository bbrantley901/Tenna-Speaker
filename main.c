
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
#define BUTTON1_PIN     (1U) // TennaTalking
#define BUTTON2_PIN     (2U) // Its Tv Time
#define BUTTON3_PIN     (3U) // Tenna Beep
#define BUTTON4_PIN     (4U) // Crowd Cheer
#define BUTTON5_PIN     (5U) // Crowd Laughter
#define BUTTON6_PIN     (6U) // Crowd Gasp
#define I2S_DATA_PIN    (19U)
#define I2S_BCLK_PIN    (20U)
#define I2S_LRCLK_PIN   (21U)

#define BUTTONS_GPIO_MASK ((1 << BUTTON1_PIN) | \
                           (1 << BUTTON2_PIN) | \
                           (1 << BUTTON3_PIN) | \
                           (1 << BUTTON4_PIN) | \
                           (1 << BUTTON5_PIN) | \
                           (1 << BUTTON6_PIN))

#if 0 //Pre refactor
/* Cast uint8_t array to int16_t */
static const int16_t *audio_samples = (const int16_t *)tenna_talking;

/* Sample Size is array length / 2 ; 2 bytes/sample*/
static const size_t audio_sample_count = tenna_talking_len / 2;
#endif 

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
static uint32_t volume = 32;

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
    gpio_set_irq_enabled_with_callback(BUTTON1_PIN, GPIO_IRQ_EDGE_FALL, true, change_audio_file_callback);
    for (int i = 1; i <= 6; i++) {
        gpio_pull_up(i);
        gpio_set_irq_enabled(i, GPIO_IRQ_EDGE_FALL, true);
    }

    /* Event Loop*/
    while(1)
    {   
# if (1)
      /* Button Audio Playback */
        if (audio_queued) {
/* With this current configuration, an audio file will play in its entirety before 
   the next interrupt can change it. I can keep this as is but if I want to change audio
   mid playback then I need to edit play_audio to return if the IRQ activates during execution */
            play_audio(current_audio_file->samples, current_audio_file->sample_count); 
            audio_queued = false; 
        }
#else
        /* Continuous Audio Playback for Demo Purposes */
        play_audio(current_audio_file->samples, current_audio_file->sample_count);
        sleep_ms(2000);
#endif
    }

#if 0 //Pre refactor
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
          int16_t s = audio_samples[pos++];
          samples[i] = s;
          //samples[i+1] = s; 
          if (pos >= sample_count) {
            buffer->sample_count = i; //Only process the samples we have
            break; // No more samples left; exit allocation of audio buffer
          }
        }
        buffer->sample_count = count;
        give_audio_buffer(ap, buffer); // Play audio buffer
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
  if (!gpio_get(BUTTON1_PIN)) {
      current_audio_file = &audio_file_list[0];
  } else if (!gpio_get(BUTTON2_PIN)) {
      current_audio_file = &audio_file_list[1];
  } else if (!gpio_get(BUTTON3_PIN)) {
      current_audio_file = &audio_file_list[2];
  } else if (!gpio_get(BUTTON4_PIN)) {
      current_audio_file = &audio_file_list[3];
  } else if (!gpio_get(BUTTON5_PIN)) {
      current_audio_file = &audio_file_list[4];
  } else if (!gpio_get(BUTTON6_PIN)) {
      current_audio_file = &audio_file_list[5];
  }
  audio_queued = true; 
}