/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "mapping.h"

#if MAPPING_LAYAOUT

extern MIDI_NAMESPACE::SerialMIDI<HardwareSerial>serialMIDI;
extern MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDI;

#define GRID_COLS   14
#define GRID_ROWS   10
#define GRID_KEYS   (GRID_COLS * GRID_ROWS)
#define GRID_GAP    1
#define KEY_SIZE_X  (uint8_t)((X_MAX - (GRID_GAP * (GRID_COLS + 1))) / GRID_COLS)
#define KEY_SIZE_Y  (uint8_t)((Y_MAX - (GRID_GAP * (GRID_ROWS + 1))) / GRID_ROWS)

#define GRID_COMPUTE_ROW_PTR(grid_ptr, y) (((squareKey_t *)(grid_ptr)) + ((GRID_COLS * sizeof(squareKey_t)) * y));
#define GRID_GET_KEY_PTR(col_ptr, x)(col_ptr + x);

#define SWITCH_DEBOUNCE_TIME    15 // TO_REMOVE: this have been done in find_blobs();

squareKey_t keyPos[GRID_KEYS];                  // 1D array of struct squareKey_t to store pre-compute key positions ARGS[Xmin, Xmax, Ymin, Ymax]
squareKey_t* lastKeyPress_ptr[MAX_SYNTH];       // 1D array to store last keys pressed

uint8_t freqKeyLayout[GRID_KEYS] = {0};         // 1D array to mapp freq
midiNode_t midiKeyLayout[GRID_KEYS] = {0};      // 1D array to mapp incoming midi notes in the grid layout

void mapping_trigger(tSwitch_t* switch_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > switch_ptr->posX - switch_ptr->rSize &&
        blob_ptr->centroid.X < switch_ptr->posX + switch_ptr->rSize) {
      if (blob_ptr->centroid.Y > switch_ptr->posY - switch_ptr->rSize &&
          blob_ptr->centroid.Y < switch_ptr->posY + switch_ptr->rSize) {
        if (millis() - switch_ptr->timeStamp > SWITCH_DEBOUNCE_TIME) {
          switch_ptr->timeStamp = millis();
          switch_ptr->state = true;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TRIGGER : POSX: % f\tPOSY: % f", blob_ptr->centroid.X, blob_ptr->centroid.Y);
#endif
          //return true;
        };
        switch_ptr->timeStamp = millis();
      };
    }
    else {
      //return false;
    };
  };
};

void mapping_toggle(tSwitch_t* switch_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > switch_ptr->posX - switch_ptr->rSize &&
        blob_ptr->centroid.X < switch_ptr->posX + switch_ptr->rSize) {
      if (blob_ptr->centroid.Y > switch_ptr->posY - switch_ptr->rSize &&
          blob_ptr->centroid.Y < switch_ptr->posY + switch_ptr->rSize) {
        if (millis() - switch_ptr->timeStamp > SWITCH_DEBOUNCE_TIME) {
          switch_ptr->timeStamp = millis();
          switch_ptr->state = !switch_ptr->state;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_TOGGLE : STATE: % d", switch_ptr->state);
#endif
          //return switch_ptr->state;
        };
        switch_ptr->timeStamp = millis();
      };
    };
  };
};

// Pre-compute key min & max coordinates
void GRID_LAYOUT_SETUP(void) {
  Serial.printf("\nKEY_SIZE_X:%d \tKEY_SIZE_Y:%d", KEY_SIZE_X, KEY_SIZE_Y);
  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      uint8_t index = row * GRID_COLS + col;
      keyPos[index].val = index;
      //keyPos[index].val = (int8_t)harmonicKeyboard[index];
      keyPos[index].Xmin = col * KEY_SIZE_X + ((col + 1) * GRID_GAP);
      keyPos[index].Xmax = keyPos[index].Xmin + KEY_SIZE_X;
      keyPos[index].Ymin = row * KEY_SIZE_Y + ((row + 1) * GRID_GAP);
      keyPos[index].Ymax = keyPos[index].Ymin + KEY_SIZE_Y;
#if DEBUG_MAPPING
      Serial.printf("\nGRID\tXmin:%d\tXmax:%d\tYmin:%d\tYmax:%d",
                    keyPos[index].Xmin, keyPos[index].Xmax,
                    keyPos[index].Ymin, keyPos[index].Ymax
                   );
#endif
    };
  };
};

// Compute the grid index location acording to the blobs XY (centroid) coordinates
// Play corresponding midi **note** or **freq**
void mapping_gridPlay(void) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->UID < MAX_SYNTH) {                                          // Test if the blob UID is less than MAX_SYNTH
      int keyPosX = round((blob_ptr->centroid.X / (float)X_MAX) * GRID_COLS); // Compute X window position
      int keyPosY = round((blob_ptr->centroid.Y / (float)Y_MAX) * GRID_ROWS); // Compute Y window position
      int index = (keyPosY * GRID_COLS) + keyPosX;                            // Compute 1D key index position
      //Serial.printf("\nGRID\tBLOB:%d\tSTATE:%d\tLAST_STATE:%d\tKEY:%d", blob_ptr->UID, blob_ptr->state, blob_ptr->lastState, index);
      squareKey_t* keyPress_ptr = &keyPos[index];
      // Test if the blob is within the key limits
      if (blob_ptr->centroid.X > keyPress_ptr->Xmin && blob_ptr->centroid.X < keyPress_ptr->Xmax &&
          blob_ptr->centroid.Y > keyPress_ptr->Ymin && blob_ptr->centroid.Y < keyPress_ptr->Ymax) {
        if (blob_ptr->state) {
          if (!blob_ptr->lastState) {
#if DEBUG_MAPPING
            Serial.printf("\nGRID\tBLOB:%d\t\tKEYDOWN:%d", blob_ptr->UID, (uint8_t)keyPress_ptr->val);
#else
            MIDI.sendNoteOn(keyPress_ptr->val, 127, 1);                   // Send NoteON (CHANNEL_1)
#endif
          };
        }
        else {
#if DEBUG_MAPPING
          Serial.printf("\nGRID\tBLOB:%d\t\tKEYUP:%d", blob_ptr->UID, (uint8_t)keyPress_ptr->val);
#else
          MIDI.sendNoteOff(lastKeyPress_ptr[blob_ptr->UID]->val, 0, 1);  // Send NoteOFF (CHANNEL_1)
#endif
        };
      };
    };
  };
};

void mapping_gridPopulate(void) {
  uint8_t keyIndex = 0;
  while (keyIndex < GRID_KEYS) {
    for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&blobs); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
      midiKeyLayout[keyIndex].pithch = node_ptr->pithch;
      midiKeyLayout[keyIndex].velocity = node_ptr->velocity;
      midiKeyLayout[keyIndex].channel = node_ptr->channel;
      keyIndex++;
    };
  };
};

void mapping_vSlider(vSlider_t* slider_ptr) {
  uint8_t val = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.X > slider_ptr->posX - slider_ptr->width &&
        blob_ptr->centroid.X < slider_ptr->posX + slider_ptr->width) {
      if (blob_ptr->centroid.Y > slider_ptr->Ymin &&
          blob_ptr->centroid.Y < slider_ptr->Ymax) {
        val = round(map(blob_ptr->centroid.Y, slider_ptr->Ymin, slider_ptr->Ymax, 0, 127)); // [0:127]
        if (val != slider_ptr->val) {
          slider_ptr->val = val;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_V_SLIDER : % d", val);
#endif
        };
      };
    };
  };
};

void mapping_hSlider(hSlider_t* slider_ptr) {
  uint8_t val = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    if (blob_ptr->centroid.Y > slider_ptr->posY - slider_ptr->height &&
        blob_ptr->centroid.Y < slider_ptr->posY + slider_ptr->height) {
      if (blob_ptr->centroid.X > slider_ptr->Xmin &&
          blob_ptr->centroid.X < slider_ptr->Xmax) {
        val = round(map(blob_ptr->centroid.X, slider_ptr->Xmin, slider_ptr->Xmax, 0, 127)); // [0:127]
        if (val != slider_ptr->val) {
          slider_ptr->val = val;
#if DEBUG_MAPPING
          Serial.printf("\nDEBUG_H_SLIDER : % d", val);
#endif
        };
      };
    };
  };
};

void mapping_cSlider(cSlider_t* slider_ptr) {
  float phi = 0;
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    for (int i = 0; i < C_SLIDERS; i++) {
      if (blob_ptr->polar.r > slider_ptr[i].r - slider_ptr[i].width &&
          blob_ptr->polar.r < slider_ptr[i].r + slider_ptr[i].width) {
        if (blob_ptr->polar.phi > slider_ptr[i].phiOffset) {
          phi = blob_ptr->polar.phi - slider_ptr[i].phiOffset;
        }
        else {
          phi = blob_ptr->polar.phi + (PI2 - slider_ptr[i].phiOffset);
        }
#if DEBUG_MAPPING
        Serial.printf("\nDEBUG_C_SLIDER_ % d phi : % f", i, map(constrain(phi, 0.2, 5.9), 0.2, 5.9, 0, 127));
#endif
      };
    };
  };
};


// ccPesets_ptr -> ARGS[blobID, [BX,BY,BW,BH,BD], cChange, midiChannel, Val]
void mapping_controlChange(ccPesets_t* ccPesets_ptr) {
  for (blob_t* blob_ptr = (blob_t*)ITERATOR_START_FROM_HEAD(&blobs); blob_ptr != NULL; blob_ptr = (blob_t*)ITERATOR_NEXT(blob_ptr)) {
    // Test if we are within the blob limit
    if (blob_ptr->UID == ccPesets_ptr->blobID) {
      // Test if the blob is alive
      if (blob_ptr->state) {
        switch (ccPesets_ptr->mappVal) {
          case BX:
            if (blob_ptr->centroid.X != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.X;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.X, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.X, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
          case BY:
            if (blob_ptr->centroid.Y != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->centroid.Y;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->centroid.Y, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->centroid.Y, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
          case BW:
            if (blob_ptr->box.W != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.W;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.W, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.W, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
          case BH:
            if (blob_ptr->box.H != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.H;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nMIDI\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.H, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
          case BD:
            if (blob_ptr->box.D != ccPesets_ptr->val) {
              ccPesets_ptr->val = blob_ptr->box.D;
#if DEBUG_MIDI_HARDWARE
              Serial.printf("\nBLOB:%d\tCC:%d\tVAL:%d\tCHAN:%d", blob_ptr->UID, ccPesets_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), ccPesets_ptr->midiChannel);
#else
              MIDI.sendControlChange(ccPesets_ptr->cChange, constrain(blob_ptr->box.D, 0, 127), ccPesets_ptr->midiChannel);
#endif
            }
            break;
        }
      }
    }
  }
}
#endif
