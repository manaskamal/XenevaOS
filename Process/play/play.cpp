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
#include <audio/audio.h>


void drawAnim() {
	int i = 0;
	while (1) {
		printf("\r");
		printf("Animation :%d", i++);
		_KeProcessSleep(10000);
	}
}
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

	int postbox = _KeOpenFile("/dev/postbox", FILE_OPEN_READ_ONLY);
	_KeFileIoControl(postbox, POSTBOX_CREATE, NULL);
	printf("\n");
	
	DeodhaiAudioBox* audioBox = DeodhaiAudioOpenConnection(postbox, DEODHAI_AUDIO_STEREO, DEODHAI_CONNECTION_TYPE_NORMAL);
	printf("play: audio connection initiated successfully \n");


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
	
	while (1) {
		/* with each frame read the sound, write it
		* to sound device, the sound device will automatically
		* put the app to sleep for smooth playback for some
		* milli-seconds
		*/
		_KeFileStat(song, &fs);

		if (fs.eof) {
			finished = 1;
			DeodhaiAudioCloseConnection(audioBox);
			_KeCloseFile(song);
			_KeProcessExit();
		}

		if (!finished) {
			//_KeWriteFile(snd, songbuf, 4096);
			if (!audioBox->ctlPanel->Samplefull) {
				_KeReadFile(song, songbuf, 4096);
				DeodhaiAudioWrite(audioBox, songbuf);
			}
			else {
				_KeProcessSleep(120);
			}
		}
		_KeProcessSleep(10);
	}
}