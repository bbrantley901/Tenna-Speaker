#!/usr/bin/env bash
set -euo pipefail

ERROR_LEVEL=0


function audio_to_pcm() {
  OUTPUT_PCM_FILE="temp.raw"
  echo "Converting $INPUT_AUDIO_FILE to PCM format and saving as $OUTPUT_PCM_FILE"
  ffmpeg -i "$INPUT_AUDIO_FILE" -ac 1 -ar 44100 -f s16le -acodec pcm_s16le "$OUTPUT_PCM_FILE"
  ERROR_LEVEL=$?
  if [ $ERROR_LEVEL -ne 0 ]; then
    echo "Failed to convert audio to PCM, exit status: $ERROR_LEVEL"
    exit $ERROR_LEVEL
  fi
}

function pcm_to_header() {
  INPUT_PCM_FILE="temp.raw"
  echo "Converting $INPUT_PCM_FILE to C header file $OUTPUT_HEADER_FILE"
  xxd -i "$INPUT_PCM_FILE" > "$OUTPUT_HEADER_FILE"
  ERROR_LEVEL=$?
  if [ $ERROR_LEVEL -ne 0 ]; then
    echo "Failed to convert PCM to header file, exit status: $ERROR_LEVEL"
    exit $ERROR_LEVEL
  fi
  echo "Moving header file to inc/ directory and cleaning up temp files"
  mv "$OUTPUT_HEADER_FILE" inc/
  rm "$INPUT_PCM_FILE"
}

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <input_audio_file> <output_header_file>"
  exit 1
fi
INPUT_AUDIO_FILE="$1"
OUTPUT_HEADER_FILE="$2"

audio_to_pcm "$1"
pcm_to_header "$2"
echo "Audio conversion to header file completed successfully"
