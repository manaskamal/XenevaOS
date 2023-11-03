/**
* BSD 2-Clause License
*
* Copyright (c) 2021, Manas Kamal Choudhury
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
**/

#ifndef __AU_SOUND_H__
#define __AU_SOUND_H__

#include <stdint.h>
#include <circbuf.h>
#include <Hal\x86_64_sched.h>
#include <aurora.h>

#pragma pack(push,1)
typedef struct __au_dsp__ {
	CircBuffer *buffer;
	uint16_t _dsp_id;
	AuThread *SndThread;
	uint64_t sleep_time;
	bool available;
	struct __au_dsp__ *next;
	struct __au_dsp__ *prev;
}AuDSP;
#pragma pack(pop)


typedef struct _SoundDev_ {
	char name[32];
	void(*write) (uint8_t* buffer, size_t length);
	void(*read) (uint8_t* buffer, size_t length);
	void(*stop_output)();
	void(*start_output)();
	void(*set_vol)(uint8_t vol);
}AuSound;


/*
* AuSoundInitialize -- Initialized the Aurora sound system
*/
extern void AuSoundInitialise();

/*
* AuSoundSetCard -- registers a new
* sound card
* @param dev -- sound card device
*/
AU_EXTERN AU_EXPORT void AuSoundSetCard(AuSound* dev);

/*
* AuSoundGetBlock -- the main heart of aurora sound
* system -- called by sound card
*/
AU_EXTERN AU_EXPORT void AuSoundGetBlock(uint64_t *buffer);

/*
* AuSoundRemoveDSP -- remove the dsp from
* dsp list
*/
AU_EXTERN AU_EXPORT void AuSoundRemoveDSP(uint16_t id);

/*
* AuSoundStart -- Starts the Sound card
*/
extern void AuSoundStart();

/*
* AuSoundStop -- Stop Sound card
*/
extern void AuSoundStop();
#endif