/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "config.h"
#include "presets.h"
#include "llist.h"
#include "blob.h"
#include "mapping.h"

#include <Audio.h>              // https://github.com/PaulStoffregen/Audio
#include <Wire.h>               // https://github.com/PaulStoffregen/Wire
#include <SPI.h>                // https://github.com/PaulStoffregen/SPI
#include <SD.h>                 // https://github.com/PaulStoffregen/SD
#include <SerialFlash.h>        // https://github.com/PaulStoffregen/SerialFlash

typedef struct preset preset_t;     // Forward declaration
typedef struct llist llist_t;       // Forward declaration
typedef struct blob blob_t;         // Forward declaration
typedef struct tSwitch tSwitch_t;   // Forward declaration
typedef struct cSlider cSlider_t;   // Forward declaration

typedef struct synth {
  AudioSynthWaveform* wf_ptr;
  AudioSynthWaveformSineModulated* fm_ptr;
  AudioEffectFade* fade_ptr;
  AudioMixer4* mix_ptr;
  uint8_t lastBlobState;
} synth_t;

void SETUP_DAC(
  AudioControlSGTL5000* dac_ptr,
  preset_t* presets_ptr,
  synth_t* synths_ptr
);

void set_volumes(
  AudioControlSGTL5000* dac_ptr,
  preset_t* presets_ptr
);

void make_noise(
  AudioControlSGTL5000* dac_ptr,
  llist_t* blobs_ptr,
  synth_t* allSynth_ptr
);

void stepSequencer();

#endif /*__AUDIO_H__*/
