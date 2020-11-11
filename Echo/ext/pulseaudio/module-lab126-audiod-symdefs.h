/***
  This file is part of PulseAudio.

  Copyright (c) 2013 Amazon.com, Inc. or its affiliates. All Rights Reserved

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
#ifndef modulelab126audiodsymdef
#define modulelab126audiodsymdef

#include <pulsecore/core.h>
#include <pulsecore/module.h>
#include <pulsecore/macro.h>

/// Standard Pulseaudio definitions and API functions

#define pa__init            module_lab126_audiod_LTX_pa__init
#define pa__done            module_lab126_audiod_LTX_pa__done
#define pa__get_author      module_lab126_audiod_LTX_pa__get_author
#define pa__get_description module_lab126_audiod_LTX_pa__get_description
#define pa__get_usage       module_lab126_audiod_LTX_pa__get_usage
#define pa__get_version     module_lab126_audiod_LTX_pa__get_version
#define pa__get_deprecated  module_lab126_audiod_LTX_pa__get_deprecated
#define pa__load_once       module_lab126_audiod_LTX_pa__load_once
#define pa__get_n_used      module_lab126_audiod_LTX_pa__get_n_used

int pa__init(pa_module*m);
void pa__done(pa_module*m);
int pa__get_n_used(pa_module*m);

const char* pa__get_author(void);
const char* pa__get_description(void);
const char* pa__get_usage(void);
const char* pa__get_version(void);
const char* pa__get_deprecated(void);
pa_bool_t pa__load_once(void);

#endif // modulelab126audiodsymdef
