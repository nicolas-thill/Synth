/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __PLAYER_FLASH_H__
#define __PLAYER_FLASH_H__

#include <Audio.h>               // https://github.com/PaulStoffregen/Audio
#include "config.h"
#include "llist.h"
#include "blob.h"
#include "mapping.h"

typedef struct llist llist_t;   // Forward declaration
typedef struct blob blob_t;     // Forward declaration

void FLASH_PLAYER_SETUP(void);
void flash_player(llist_t* llist_ptr, AudioPlaySerialflashRaw* player_ptr);

#endif /*__PLAYER_FLASH__*/