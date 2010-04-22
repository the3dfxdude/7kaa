/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010 Enlight Software Ltd. and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef OAUDIO_H
#define OAUDIO_H

#include <win32_audio.h>
#include <openal_audio.h>

#ifndef AUDIO_BACKEND
#error "You need to define an audio backend, such as OpenAL or Win32"
#endif

#define AUDIO_BACKEND_CLASS_NAME2(backend) backend##Audio
#define AUDIO_BACKEND_CLASS_NAME(backend) AUDIO_BACKEND_CLASS_NAME2(backend)

typedef AUDIO_BACKEND_CLASS_NAME(AUDIO_BACKEND) Audio;

extern Audio audio;

#endif
