/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "presets.h"

// HARDWARE CONSTANTES **DO NOT CHANGE**
#define LED_PIN_D1              5
#define LED_PIN_D2              4
#define BUTTON_PIN_L            2
#define BUTTON_PIN_R            3
#define ENCODER_PIN_A           22
#define ENCODER_PIN_B           9

// SOFTWARE CONSTANTES **DO NOT CHANGE**
#define LONG_HOLD                   1500
#define BLOBS_PLAY_LED_TIMEON       600
#define BLOBS_PLAY_LED_TIMEOFF      600
#define BLOBS_LEARN_LED_TIMEON      100
#define BLOBS_LEARN_LED_TIMEOFF     100
#define BLOBS_MAPPING_LED_TIMEON    1000
#define BLOBS_MAPPING_LED_TIMEOFF   100
#define CALIBRATE_LED_TIMEON        35
#define CALIBRATE_LED_TIMEOFF       100
#define CALIBRATE_LED_ITER          5
#define SAVE_LED_TIMEON             20
#define SAVE_LED_TIMEOFF            50
#define SAVE_LED_ITER               10

Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
//Button BUTTON_L = Button(); // DEPRECATED
Bounce2::Button BUTTON_L = Bounce2::Button();
//Button BUTTON_R = Button(); // DEPRECATED
Bounce2::Button BUTTON_R = Bounce2::Button();

uint8_t currentMode = CALIBRATE;      // Init currentMode with CALIBRATE (SET as DEFAULT_MODE)
//uint8_t lastMode = LINE_OUT;        // Init lastMode with LINE_OUT (SET as DEFAULT_MODE)
uint8_t lastMode = BLOBS_MAPPING;     // Init lastMode with MIDI_MAPPING (SET as DEFAULT_MODE)
//uint8_t lastMode = BLOBS_PLAY;      // Init lastMode with MIDI_BLOBS_PLAY (SET as DEFAULT_MODE)

preset_t presets[9] = {
  {13, 31, 29, 0, false, false, false, false, LOW,  LOW  },  // LINE_OUT          ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 1, 50, 12, 0, false, false, false, false, HIGH, LOW  },  // SIG_IN            ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 1, 31, 17, 0, false, false, false, false, LOW,  HIGH },  // SIG_OUT           ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 2, 60, 20, 0, false, false, false, false, HIGH, HIGH },  // THRESHOLD         ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 0, 0,  0,  0, true,  false, false, false, HIGH, HIGH },  // CALIBRATE         ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 0, 0,  0,  0, false, false, false, false, NULL, NULL },  // SAVE              ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 0, 0,  0,  0, false, false, false, false, NULL, NULL },  // MIDI_BLOBS_PLAY   ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 1, 7,  0,  0, false, false, false, false, NULL, NULL },  // MIDI_BLOBS_LEARN  ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
  { 0, 0,  0,  0, false, false, false, false, NULL, NULL }   // MIDI_MAPPING      ARGS[minVal, maxVal, val, ledVal, update, setLed, updateLed, allDone, D1, D2]
};

void LEDS_SETUP(void) {
  pinMode(LED_PIN_D1, OUTPUT);
  pinMode(LED_PIN_D2, OUTPUT);
};

// Hear it should not compile if you didn't install the library (Manually!)
// [Bounce2](https://github.com/thomasfredericks/Bounce2)
// Install it in /Documents/Arduino/library/
void SWITCHES_SETUP(void) {
  BUTTON_L.attach(BUTTON_PIN_L, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_R.attach(BUTTON_PIN_R, INPUT_PULLUP);  // Attach the debouncer to a pin with INPUT_PULLUP mode
  BUTTON_L.interval(25);                        // Debounce interval of 25 millis
  BUTTON_R.interval(25);                        // Debounce interval of 25 millis
};

void update_presets_usb(void) {
  for (midiNode_t* node_ptr = (midiNode_t*)ITERATOR_START_FROM_HEAD(&midiIn); node_ptr != NULL; node_ptr = (midiNode_t*)ITERATOR_NEXT(node_ptr)) {
    switch (node_ptr->midiMsg.status) {
      case midi::NoteOn:
        // NOT_USED
        break;
      case midi::NoteOff:
        // NOT_USED
        break;
      case midi::ControlChange:
        switch (node_ptr->midiMsg.data1) {
          case LINE_OUT: // PROGRAM 0
            currentMode = LINE_OUT;
            presets[LINE_OUT].update = true;
            break;
          case SIG_IN: // PROGRAM 1
            currentMode = SIG_IN;
            presets[SIG_IN].update = true;
            break;
          case SIG_OUT: // PROGRAM 2
            currentMode = SIG_OUT;
            presets[SIG_OUT].update = true;
            break;
          case THRESHOLD: // PROGRAM 3
            currentMode = THRESHOLD;
            presets[THRESHOLD].val = map(node_ptr->midiMsg.data2, 0, 127, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
            encoder.write(presets[THRESHOLD].val << 2);
            interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
            presets[THRESHOLD].ledVal = map(node_ptr->midiMsg.data2, 0, 127, 0, 255);
            presets[THRESHOLD].update = true;
            presets[THRESHOLD].setLed = true; // TEENSY 3.2 DO NOT HAVE LEDs!!!
            break;
          case CALIBRATE: // PROGRAM 4
            lastMode = currentMode;
            currentMode = CALIBRATE;
            presets[CALIBRATE].update = true;
            break;
          case BLOBS_PLAY: // PROGRAM 6
            currentMode = BLOBS_PLAY;
            presets[BLOBS_PLAY].update = true;
            presets[BLOBS_PLAY].setLed = true;
            break;
          case BLOBS_LEARN: // PROGRAM 7
            currentMode = BLOBS_LEARN;
            presets[BLOBS_LEARN].update = true;
            presets[BLOBS_LEARN].setLed = true;
            break;
          case BLOBS_MAPPING: // PROGRAM 8
            currentMode = BLOBS_MAPPING;
            presets[BLOBS_MAPPING].update = true;
            presets[BLOBS_MAPPING].setLed = true;
            break;
          case RAW_MATRIX: // PROGRAM 9
            currentMode = RAW_MATRIX;
            //presets[RAW_MATRIX].updat = true;
            break;
          case INTERP_MATRIX: // PROGRAM 10
            currentMode = INTERP_MATRIX;
            //presets[INTERP_MATRIX].update = true;
            break;
          case ALL_OFF: // PROGRAM 11
            currentMode = ALL_OFF;
            //presets[ALL_OFF].update = true;
            break;
          default:
            break;
        };
        break;
      default:
        break;
    };
  };
  llist_save_nodes(&midi_node_stack, &midiIn); // Save/rescure all midiOut nodes
};

void update_presets_buttons(void) {
  BUTTON_L.update();
  BUTTON_R.update();
  // ACTION : BUTTON_L short press
  // FONCTION : CALIBRATE THE SENSOR MATRIX
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() < LONG_HOLD) {
    lastMode = currentMode; // keep track of last Mode to set it back after calibration
    currentMode = CALIBRATE;
    presets[CALIBRATE].update = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_L : CALIBRATE : %d", currentMode);
#endif
  };
  // ACTION : BUTTON_L long press
  // FONCTION : SAVE ALL PARAMETERS TO THE EEPROM MEMORY
  if (BUTTON_L.rose() && BUTTON_L.previousDuration() > LONG_HOLD) {
    lastMode = currentMode; // keep track of last Mode to set it back after saving
    currentMode = SAVE;
    presets[SAVE].update = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_L : SAVE : %d", currentMode);
#endif
  };
  // ACTION : BUTTON_R short press
  // FONCTION : SELECT_MODE
  // [0]-LINE_OUT
  // [1]-SIG_IN
  // [2]-SIG_OUT
  // [3]-THRESHOLD
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() < LONG_HOLD) {
    currentMode = (currentMode + 1) % 4;   // Loop into the modes
    encoder.write(presets[currentMode].val << 2);
    presets[currentMode].setLed = true;
#if DEBUG_BUTTONS
    Serial.printf("\nBUTTON_R : SELECT_MODE : %d", currentMode);
#endif
  };
  // ACTION : BUTTON_R long press
  // FONCTION_A : MIDI_BLOBS_PLAY (send all blob values over MIDI format)
  // FONCTION_B : MIDI_BLOBS_LEARN (send blob values separately for Max4Live MIDI_LEARN)
  // LEDs : blink alternately, slow for playing mode and fast or learning mode
  if (BUTTON_R.rose() && BUTTON_R.previousDuration() > LONG_HOLD) {
    if (currentMode == BLOBS_LEARN) {
      currentMode = BLOBS_LEARN;
      presets[BLOBS_LEARN].update = true;
#if DEBUG_BUTTONS
      Serial.printf("\nMIDI_BLOBS_PLAY : %d", currentMode);
#endif
    }
    else {
      currentMode = BLOBS_PLAY;
      encoder.write(0x1);
      presets[BLOBS_PLAY].update = true;
#if DEBUG_BUTTONS
      Serial.printf("\nMIDI_BLOBS_LEARN : %d", currentMode);
#endif
    };
  };
};

// Update preset of each mode with the encoder position
void update_presets_encoder(void) {
  switch (currentMode) {
    case LINE_OUT:
      if (setLevel(&presets[LINE_OUT])) {
        presets[LINE_OUT].ledVal = map(presets[LINE_OUT].val, presets[LINE_OUT].minVal, presets[LINE_OUT].maxVal, 0, 255);
        presets[LINE_OUT].updateLed = true;
      };
      break;
    case SIG_IN:
      if (setLevel(&presets[SIG_IN])) {
        presets[SIG_IN].ledVal = map(presets[SIG_IN].val, presets[SIG_IN].minVal, presets[SIG_IN].maxVal, 0, 255);
        presets[SIG_IN].updateLed = true;
      };
      break;
    case SIG_OUT:
      if (setLevel(&presets[SIG_OUT])) {
        presets[SIG_OUT].ledVal = map(presets[SIG_OUT].val, presets[SIG_OUT].minVal, presets[SIG_OUT].maxVal, 0, 255);
        presets[SIG_OUT].updateLed = true;
      };
      break;
    case THRESHOLD:
      if (setLevel(&presets[THRESHOLD])) {
        presets[THRESHOLD].ledVal = map(presets[THRESHOLD].val, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal, 0, 255);
        interpThreshold = constrain(presets[THRESHOLD].val - 5, presets[THRESHOLD].minVal, presets[THRESHOLD].maxVal);
        presets[THRESHOLD].updateLed = true;
      };
      break;
    case BLOBS_LEARN:
      if (setLevel(&presets[BLOBS_LEARN]));
      break;
    default:
      break;
  };
};

// Update LEDs according to the mode and rotary encoder values
void update_leds(void) {
  static uint32_t timeStamp = 0;
  static uint8_t iter = 0;

  switch (currentMode) {
    case LINE_OUT:
      if (presets[LINE_OUT].setLed) {
        presets[LINE_OUT].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets[LINE_OUT].D1);
        digitalWrite(LED_PIN_D2, presets[LINE_OUT].D2);
      };
      if (presets[LINE_OUT].updateLed) {
        presets[LINE_OUT].updateLed = false;
        analogWrite(LED_PIN_D1, presets[LINE_OUT].ledVal);
        analogWrite(LED_PIN_D2, abs(presets[LINE_OUT].ledVal - 255));
      };
      break;
    case SIG_IN:
      if (presets[SIG_IN].setLed) {
        presets[SIG_IN].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets[SIG_IN].D1);
        digitalWrite(LED_PIN_D2, presets[SIG_IN].D2);
      };
      if (presets[SIG_IN].updateLed) {
        presets[SIG_IN].updateLed = false;
        analogWrite(LED_PIN_D1, presets[SIG_IN].ledVal);
        analogWrite(LED_PIN_D2, abs(presets[SIG_IN].ledVal - 255));
      };
      break;
    case SIG_OUT:
      if (presets[SIG_OUT].setLed) {
        presets[SIG_OUT].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets[SIG_OUT].D1);
        digitalWrite(LED_PIN_D2, presets[SIG_OUT].D2);
      };
      if (presets[SIG_OUT].updateLed) {
        presets[SIG_OUT].updateLed = false;
        analogWrite(LED_PIN_D1, presets[SIG_OUT].ledVal);
        analogWrite(LED_PIN_D2, abs(presets[SIG_OUT].ledVal - 255));
      };
      break;
    case THRESHOLD:
      if (presets[THRESHOLD].setLed) {
        presets[THRESHOLD].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        digitalWrite(LED_PIN_D1, presets[THRESHOLD].D1);
        digitalWrite(LED_PIN_D2, presets[THRESHOLD].D2);
      };
      if (presets[THRESHOLD].updateLed) {
        presets[THRESHOLD].updateLed = false;
        analogWrite(LED_PIN_D1, presets[THRESHOLD].ledVal);
        analogWrite(LED_PIN_D2, abs(presets[THRESHOLD].ledVal - 255));
        //lastMode = currentMode;
      };
      break;
    case CALIBRATE: // LEDs : both LED are blinking three time
      if (presets[CALIBRATE].setLed) {
        presets[CALIBRATE].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
        iter = 0;
      };
      if (presets[CALIBRATE].allDone) {
        if (iter <= CALIBRATE_LED_ITER) {
          if (millis() - timeStamp < CALIBRATE_LED_TIMEON && presets[CALIBRATE].updateLed == true ) {
            presets[CALIBRATE].updateLed = false;
            digitalWrite(LED_PIN_D1, HIGH);
            digitalWrite(LED_PIN_D2, HIGH);
          }
          else if (millis() - timeStamp > CALIBRATE_LED_TIMEON && presets[CALIBRATE].updateLed == false) {
            presets[CALIBRATE].updateLed = true;
            digitalWrite(LED_PIN_D1, LOW);
            digitalWrite(LED_PIN_D2, LOW);
          }
          else if (millis() - timeStamp > CALIBRATE_LED_TIMEON + CALIBRATE_LED_TIMEOFF) {
            if (iter < CALIBRATE_LED_ITER) {
              timeStamp = millis();
              iter++;
            }
            else {
              presets[CALIBRATE].allDone = false;
              currentMode = lastMode;
              presets[currentMode].setLed = true;
            };
          };
        };
      };
      break;
    case BLOBS_PLAY: // LEDs : blink alternately slow
      if (presets[BLOBS_PLAY].setLed) {
        presets[BLOBS_PLAY].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
      };
      if (millis() - timeStamp < BLOBS_PLAY_LED_TIMEON && presets[BLOBS_PLAY].updateLed == true) {
        presets[BLOBS_PLAY].updateLed = false;
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, LOW);
      }
      else if (millis() - timeStamp > BLOBS_PLAY_LED_TIMEON && presets[BLOBS_PLAY].updateLed == false) {
        presets[BLOBS_PLAY].updateLed = true;
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, HIGH);
      }
      else if (millis() - timeStamp > BLOBS_PLAY_LED_TIMEON + BLOBS_PLAY_LED_TIMEOFF) {
        timeStamp = millis();
      };
      break;
    case BLOBS_LEARN: // LEDs : blink alternately fast
      if (presets[BLOBS_LEARN].setLed) {
        presets[BLOBS_LEARN].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
      };
      if (millis() - timeStamp < BLOBS_LEARN_LED_TIMEON && presets[BLOBS_LEARN].updateLed == true) {
        presets[BLOBS_LEARN].updateLed = false;
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, LOW);
      }
      else if (millis() - timeStamp > BLOBS_LEARN_LED_TIMEON && presets[BLOBS_LEARN].updateLed == false) {
        presets[BLOBS_LEARN].updateLed = true;
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, HIGH);
      }
      else if (millis() - timeStamp > BLOBS_LEARN_LED_TIMEON + BLOBS_LEARN_LED_TIMEOFF) {
        timeStamp = millis();
      };
      break;
    case SAVE: // LEDs : Both LEDs are blinking very fast
      if (presets[SAVE].setLed) {
        presets[SAVE].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
        iter = 0;
      };
      if (iter <= SAVE_LED_ITER) {
        if (millis() - timeStamp < SAVE_LED_TIMEON && presets[SAVE].updateLed == true) {
          presets[SAVE].updateLed = false;
          digitalWrite(LED_PIN_D1, HIGH);
          digitalWrite(LED_PIN_D2, HIGH);
        }
        else if (millis() - timeStamp > SAVE_LED_TIMEON && presets[SAVE].updateLed == false) {
          presets[SAVE].updateLed = true;
          digitalWrite(LED_PIN_D1, LOW);
          digitalWrite(LED_PIN_D2, LOW);
        }
        else if (millis() - timeStamp > SAVE_LED_TIMEON + SAVE_LED_TIMEOFF) {
          if (iter < SAVE_LED_ITER) {
            timeStamp = millis();
            iter++;
          }
          else {
            currentMode = lastMode;
          };
        };
      };
      break;
    case BLOBS_MAPPING: // LEDs : blink alternately
      if (presets[BLOBS_MAPPING].setLed) {
        presets[BLOBS_MAPPING].setLed = false;
        pinMode(LED_PIN_D1, OUTPUT);
        pinMode(LED_PIN_D2, OUTPUT);
        timeStamp = millis();
      };
      if (millis() - timeStamp < BLOBS_MAPPING_LED_TIMEON && presets[BLOBS_MAPPING].updateLed == true) {
        presets[BLOBS_MAPPING].updateLed = false;
        digitalWrite(LED_PIN_D1, HIGH);
        digitalWrite(LED_PIN_D2, LOW);
      }
      else if (millis() - timeStamp > BLOBS_MAPPING_LED_TIMEON && presets[BLOBS_MAPPING].updateLed == false) {
        presets[BLOBS_MAPPING].updateLed = true;
        digitalWrite(LED_PIN_D1, LOW);
        digitalWrite(LED_PIN_D2, HIGH);
      }
      else if (millis() - timeStamp > BLOBS_MAPPING_LED_TIMEON + BLOBS_MAPPING_LED_TIMEOFF) {
        timeStamp = millis();
      };
      break;
    default:
      break;
  };
};

// Values adjustment using rotary encoder
boolean setLevel(preset_t* preset_ptr) {
  uint8_t val = encoder.read() >> 2;
  if (val != preset_ptr->val) {
    if (val > preset_ptr->maxVal) {
      encoder.write(preset_ptr->maxVal << 2);
      return false;
    }
    else if (val < preset_ptr->minVal) {
      encoder.write(preset_ptr->minVal << 2);
      return false;
    }
    else {
      preset_ptr->val = val;
      return true;
    };
  }
  else {
    return false;
  };
};

// TODO
void preset_load(void) {
  for (int i = 0; i < 4; i++) {
    // EEPROM.read(i, preset[i].val); // uint8_t
  };
};

// TODO
void save_presets(void) {
  for (int i = 0; i < 4; i++) {
    //EEPROM.write(i, preset[i].val); // uint8_t
  };
};
