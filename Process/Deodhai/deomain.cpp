/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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

#include <_xeneva.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <stdlib.h>
#include <sys\iocodes.h>
#include <string.h>
#include <chitralekha.h>

int main(int argc, char* arv[]) {
	_KePrint("Deodhai v1.0 running \n");
	ChPrintLibName();
	ChCanvas* canv = ChCreateCanvas(100, 100);
	_KePrint("Canvas screen_w -> %d, screen_h -> %d \n", canv->screenWidth, canv->screenHeight);
	_KePrint("Canvas pitch -> %d \n", canv->pitch);
	_KePrint("Canvas reqW-> %d , reqH -> %d \n", canv->canvasWidth, canv->canvasHeight);
	_KePrint("Playing sound -- Muruli.wav \n");

	int snd = _KeOpenFile("/dev/sound", FILE_OPEN_WRITE);
	XEFileIOControl ioctl;
	memset(&ioctl, 0, sizeof(XEFileIOControl));
	ioctl.uint_1 = 10;
	ioctl.syscall_magic = AURORA_SYSCALL_MAGIC;
	_KeFileIoControl(snd, SOUND_REGISTER_SNDPLR, &ioctl);
	
	int song = _KeOpenFile("/snd.wav", FILE_OPEN_READ_ONLY);
	void* songbuf = malloc(4096);
	memset(songbuf, 0, 4096);
	_KeReadFile(song, songbuf, 4096);
	XEFileStatus fs;
	_KeFileStat(song, &fs);

	while (1) {
		_KeWriteFile(snd, songbuf, 4096);
		_KeReadFile(song, songbuf, 4096);
		_KeFileStat(song, &fs);
		if (fs.eof) {
			_KeCloseFile(song);
			_KeProcessExit();
		}
	}
}