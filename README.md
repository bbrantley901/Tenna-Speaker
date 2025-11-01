# Tenna Costume Speaker System

A small system designed to send digital audio out and through a speaker system for my Tenna Halloween costume. <br>
The firmware was developed for use on an RP2350 (RPI Pico2). <br>

## Overview
All of the playable audio files for the system are first converted into raw PCM data using FFMPEG. I chose to use signed 16-bit samples at a 44.1kHz sample rate. The PCM data is stored as raw bytes within the flash memory (see /inc header files) for use in the program. <br>

The Pico-Extras I2S & Audio libraries are used to transmit the audio data via I2S to the MAX98357A module where it is converted into an analog audio signal and transmitted out into a 4Ω speaker. <br>

The audio file playback is triggered via button press. All buttons used are connected to internal pull-ups on the Pico2 and are pulled to GND when pressed, triggering a falling-edge GPIO interrupt. Each button triggers a different audio file to be played as well as two buttons for volume control. <br> 

An example schematic as well as my system build will be added at a later date.

## Equipment List
1. RPI Pico2 Dev Board
2. MAX98357A Breakout Board (I2S 3W Class D Amplifier)
3. 3W 4Ω Mono Speaker
4. Push Buttons
5. 3.3V Power Supply
<br>

## Dependencies
1. Pico-SDK (V2.2.0)
2. Pico-Extras
3. FFMPEG (optional but used for mp3->PCM conversion)
