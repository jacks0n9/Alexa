/***
  This file is part of PulseAudio.

  Copyright (c) 2014 Amazon.com, Inc. or its affiliates. All Rights Reserved

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/
#ifndef modulelab126audiodinterface
#define modulelab126audiodinterface
#include <stdint.h>

#define AUDIOD_API_SYNC         0x25CAB689
#define AUDIOD_API_TYPE_LEN     20

typedef struct {
    uint32_t    sync;
    char        type[AUDIOD_API_TYPE_LEN];
    uint16_t    volStream;
    uint16_t    volSpeak;
} tAudiodApiCmd;


#endif // modulelab126audiodinterface
