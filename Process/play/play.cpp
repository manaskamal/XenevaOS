/**
* BSD 2-Clause License
*
* Copyright (c) 2022, Manas Kamal Choudhury
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
**/

#include <stdint.h>
#include <_xeneva.h>
#include <stdio.h>
#include <sys\_keproc.h>
#include <sys/mman.h>
#include <sys\_kefile.h>
#include <sys\iocodes.h>
#include <string.h>
#include <stdlib.h>
#include <sys\_keipcpostbox.h>
#include <sys\socket.h>

/* Standard Deodhai Request message defined*/
#define DEODHAI_GET_AUDIO_CONNECTION "DeodhaiGetAudioConnection"
#define DEODHAI_CLOSE_AUDIO_CONNECTION "DeodhaiCloseAudioConnection"

#pragma pack(push,1)
typedef struct _deodhai_audio_msg_ {
	char message[60];
	uint16_t fromProcessId;
	uint16_t toProcessId;
}DeodhaiAudioMessage;
#pragma pack(pop)

/*
 * AudioControlPanel is the main way to
 * control each individual stream, feautres
 * like attack, release, Eq, compression,
 * reverb to be added in future
 */
#pragma pack(push,1)
typedef struct _audio_control_panel_ {
	uint8_t numChannel;
	float gain;
	float leftSpeakerScale;
	float rightSpeakerScale;
	bool Samplefull;
	bool ready;
}AudioControlPanel;
#pragma pack(pop)

/*
* main -- terminal emulator
*/
int main(int argc, char* arv[]){
	
	char *filename = NULL;
	if (argc > 1){
		if (strcmp(arv[1], "-help") == 0){
			printf("\n Play v1.0 -- A simple wave file player \n");
			printf("Copyright (C) Manas Kamal Choudhury 2024 \n");
			printf("Available commands : '-help', '-file' \n");
			printf("To play a file: \n");
			printf("      play -file /<yourfilename>.wav ");
		}

		if (strcmp(arv[1], "-file") == 0){
			if (!arv[1]){
				printf("No file specified to play \n");
				return -1;
			}
			filename = arv[2];
		}
	}

	if (filename == NULL){
		printf("\n No filename specified \n");
		return -1;
	}
	int thrID = _KeGetThreadID();

	int pipe = _KeOpenFile("/pipe/DeodhaiAudio", FILE_OPEN_READ_ONLY);
	printf("Pipe opened %d \n", pipe);

	int postbox = _KeOpenFile("/dev/postbox", FILE_OPEN_READ_ONLY);
	_KeFileIoControl(postbox, POSTBOX_CREATE, NULL);
	
	DeodhaiAudioMessage* msg = (DeodhaiAudioMessage*)malloc(sizeof(DeodhaiAudioMessage));
	strcpy(msg->message, DEODHAI_GET_AUDIO_CONNECTION);
	msg->fromProcessId = thrID;
	msg->toProcessId = 0;
	_KeWriteFile(pipe, msg,sizeof(DeodhaiAudioMessage));

	AudioControlPanel* panel;
	void* sampleBuffer;
	PostEvent e;
	while (1) {
		_KeFileIoControl(postbox, POSTBOX_GET_EVENT, &e);
		if (e.type != 0) {
			if (e.type == 11) {
				printf("DeodhaiAudio Handshake received \n");
				uint16_t controlPanelKey = e.dword;
				uint16_t sampleBufferKey = e.dword2;
				int id = _KeCreateSharedMem(controlPanelKey, 0, 0);
				int id2 = _KeCreateSharedMem(sampleBufferKey, 0, 0);
				void* controlPanelBuff = _KeObtainSharedMem(id, 0, 0);
				void* sampleBuff = _KeObtainSharedMem(id2, 0, 0);
				panel = (AudioControlPanel*)controlPanelBuff;
				sampleBuffer = sampleBuff;
				break;
			}
		}
		_KeProcessSleep(1000);
	}

	/* open the sound device-file, it is in /dev directory */
	int snd = _KeOpenFile("/dev/sound", FILE_OPEN_WRITE);

	
	/* allocate an ioctl structure where required information
	* will be putted */
	XEFileIOControl ioctl;
	memset(&ioctl, 0, sizeof(XEFileIOControl));

	/* uint_1 holds the millisecond to sleep after
	* one frame playback */
	ioctl.uint_1 = 0;

	/* most important aurora_syscall_magic number, without
	* this, iocontrol system call will not work */
	ioctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	/* register the app to sound layer, as it will create
	* a private dsp-box for this app */
	_KeFileIoControl(snd, SOUND_REGISTER_SNDPLR, &ioctl);

	/* now open your sound file, note that here demo is playing
	* a raw wave file with 48kHZ-16bit format, to play mp3 or
	* other format, one needs another conversion layer of samples */

	int song = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);
	if (song == -1){
		printf("No file found \n");
		return -1;
	}

	void* songbuf = malloc(4096);
	memset(songbuf, 0, 4096);
	_KeReadFile(song, songbuf, 4096);
	uint8_t* alignedSongBuf = (uint8_t*)songbuf;

	XEFileStatus fs;
	_KeFileStat(song, &fs);
	bool finished = 0;
	
	printf("ControlPanel gain -> %f \n", panel->gain);
	panel->ready = true;
	printf("Sample Buffer -> %x \n", sampleBuffer);
	panel->rightSpeakerScale = 1.0;
	panel->leftSpeakerScale = 0.0;
	panel->gain = 0.2;
	panel->numChannel = 2;
	while (1) {
		/* with each frame read the sound, write it
		* to sound device, the sound device will automatically
		* put the app to sleep for smooth playback for some
		* milli-seconds
		*/
		_KeFileStat(song, &fs);

		if (fs.eof) {
			finished = 1;
			panel->ready = false;
			_KeCloseFile(song);
			_KeProcessExit();
		}

		if (!finished) {
			//_KeWriteFile(snd, songbuf, 4096);
			if (!panel->Samplefull) {
				_KeReadFile(song, songbuf, 4096);
				memcpy(sampleBuffer, songbuf, 4096);
				panel->Samplefull = true;
			}
			else {
				_KeProcessSleep(500);
			}
		}
	}
}