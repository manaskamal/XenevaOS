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

/*
 * Init the root process -- is a statically linked program that
 * aurora expects to get it running at very first when system
 * get started
 */

#include <stdint.h>
#include <stdio.h>
#include <_xeneva.h>
#include <sys\_keproc.h>
#include <string.h>
#include <sys\_heap.h>
#include <sys\mman.h>
#include <sys\_kefile.h>
#include <stdlib.h>
#include <sys\iocodes.h>
#include <sys/_kecred.h>

/** Let's hardcode,credentials
 * untill we get proper login manager
 */
#define GROUP_INPUT  20
#define GROUP_VIDEO  21
#define GROUP_TTY    22
#define GROUP_AUDIO  21

/** hardcoded untill we get proper
 * login manager
 */
#define UAC_DEAMONS 40
#define UAC_NORMAL_USER 1000

int _sound;

/** Init Request msgs **/
#define INIT_REQUEST_PW_DOWN  "init.request.powerdown"
#define INIT_REQUEST_PW_REBOOT "init.request.reboot"


typedef struct _init_request_msg_ {
	char message[60];
	uint16_t fromProcessId;
	uint16_t toProcessId;
}InitRequestMsg;

void initSetupBasicEnvironmentVars() {
	setenv("HOME", "/", 1);
	setenv("USER", "xeuser", 1);
	setenv("USERNAME", "xeuser", 1);
	setenv("SHELL", "XenevaTerminal", 1);
	setenv("LANG", "en_US.UTF-8", 1); //Set it to English(United States), UTF-8
	setenv("OSNAME", "XenevaOS", 1);
}


typedef struct _sound_card_list {
	char name[32];
	int cardID;
	struct _sound_card_list* next;
}aurora_snd_card_list;


extern void SplashScreenShow();

void init_basic_gid_to_dev() {
	int fd = _KeOpenFile("/dev/graph", FILE_OPEN_READ_ONLY);
	_KeCredChangeID(fd, 0, GROUP_VIDEO);
	fd = _KeOpenFile("/dev/mice", FILE_OPEN_READ_ONLY);
	_KeCredChangeID(fd, 0, GROUP_INPUT);
	fd = _KeOpenFile("/dev/kybrd", FILE_OPEN_READ_ONLY);
	_KeCredChangeID(fd, 0, GROUP_INPUT);
	fd = _KeOpenFile("/dev/sound", FILE_OPEN_READ_ONLY);
	_KeCredChangeID(fd, 0, GROUP_AUDIO);
	_sound = fd;
}

/**
 * _play_startup_sound -- play the startup sound
 */
void _play_startup_sound() {
	if (_sound == -1) 
		return;
	
	XEFileIOControl ioctl;
	memset(&ioctl, 0, sizeof(XEFileIOControl));


	/* uint_1 holds the millisecond to sleep after
	* one frame playback */
	ioctl.uint_1 = 0;
	ioctl.syscall_magic = AURORA_SYSCALL_MAGIC;
	/** uint_2 must hold the sound card number to use **/
	ioctl.uint_2 = -1;
	int num_card_count = _KeFileIoControl(_sound, SOUND_GET_CARD_TOTALNUM, &ioctl);

	ioctl.uint_1 = num_card_count;
	aurora_snd_card_list* list = (aurora_snd_card_list*)malloc(sizeof(aurora_snd_card_list) * num_card_count);
	ioctl.ulong_1 = (uint64_t)list;
	if (_KeFileIoControl(_sound, SOUND_GET_CARD_LIST, &ioctl)) {
		_KePrint("[init]: failed to get sound card list \r\n");
		_KePauseThread();
	}

	/** just print all card name once **/
	for (int i = 0; i < num_card_count; i++) {
		_KePrint("[init]: %s sound is installed, id : %d \r\n", list[i].name, list[i].cardID);
	}

	/** let's use default first sound card here **/
	ioctl.uint_2 = list->cardID;

	_KeFileIoControl(_sound, SOUND_REGISTER_SNDPLR, &ioctl);

	int song = _KeOpenFile("/Ss.wav", FILE_OPEN_READ_ONLY);
	if (song == -1) {
		return;
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
			return;
		}

		if (!finished) {
			_KeReadFile(song, songbuf, 4096);
			_KeWriteFile(_sound, songbuf, 4096);
		}
		_KeProcessSleep(4);
	}
}

/**
 * @brief _init_handle_request -- handle request
 * msgs
 */
void _init_handle_request(InitRequestMsg* msg) {
	if (strcmp(msg->message, INIT_REQUEST_PW_DOWN) == 0) {
		_KePrint("[init]: shutting down xeneva \r\n");
		//clean up all init resources

		/* here also , we need to verify if the request
		 * came from active-local user id, or else deny request
		 */
		_KePowerDown();
		// no return
	}

	if (strcmp(msg->message, INIT_REQUEST_PW_REBOOT) == 0) {
		_KePrint("[init]: restarting xeneva \r\n");
		/* here also , we need to verify if the request
		 * came from active-local user id, or else deny request
		 */
		_KePowerReset();
		// no return
	}
}

/*
 * _main -- main entry point
 */
extern "C" void main(int argc, char* argv[]) {

	int pid = _KeGetProcessID();

	_KePrint("Init Process running ii %d\n", pid);
	int threadID = _KeGetThreadID();
	_KePrint("ThreadID : %d \n", threadID);

	if (strcmp(argv[0], "-about") == 0) {
#ifdef ARCH_X64
		_KePrint("Xeneva v1.0 x86_64 !! Copyright (C) Manas Kamal Choudhury 2020-2023 \r\n");
#elif ARCH_ARM64
		_KePrint("Xeneva v1.0 ARM64 !! Copyright (C) Manas Kamal Choudhury 2020-2025 \r\n");
#endif
	}

	SplashScreenShow();
	_sound = -1;
	init_basic_gid_to_dev();

	/** play the startup sound, for better experience */
	_play_startup_sound();

	int ggid_misc_world = _KeGetGlobalGroupID(AURORA_GID_MISC_WORLD);
	int ggid_misc_postbox = _KeGetGlobalGroupID(AURORA_GID_IPC_POSTBOX);

	/* open up a pipe, to get request from normal application */
	int pipe = _KeCreatePipe("init", (sizeof(InitRequestMsg) * 4));
	if (pipe == -1)
		_KePrint("[init]: pipe creation failed \r\n");
	else
		_KeCredChangeID(pipe,0, ggid_misc_world);

	/** allocate a memory for init request msgs */
	char* init_msg_buff = (char*)malloc(sizeof(InitRequestMsg) + 1);
	memset(init_msg_buff, 0, sizeof(InitRequestMsg) + 1);


	/** TODO: add IPC system to track real system progress and animate the logo accordingly **/
	_KeProcessSleep(100);

	


#ifdef ARCH_ARM64
	int proc = _KeCreateProcess(0, "netmngr");
	_KeSetUID(proc, UAC_DEAMONS);
	_KeSetGID(proc, UAC_DEAMONS);
	_KeCredAddSGroup(proc, ggid_misc_world);
	_KeProcessLoadExec(proc, "/netmngr.exe\0", 0, NULL);

	_KeProcessSleep(500);


	/** actually, design should be like that, each process after
	 * finish its initialization, it should send a signal to 
	 * init, so that it can continue next proccesses spawning
	 */


	/** from now, normal user's won't get system access */
	proc = _KeCreateProcess(0, "deodhaixr");
	_KeSetUID(proc, UAC_NORMAL_USER);
	_KeSetGID(proc, UAC_NORMAL_USER);
	_KeCredAddSGroup(proc, ggid_misc_world);
	_KeCredAddSGroup(proc, ggid_misc_postbox);
	_KeCredAddSGroup(proc, GROUP_VIDEO);
	_KeCredAddSGroup(proc, GROUP_INPUT);
	_KeCredSetCap(proc, 0);
	_KeProcessLoadExec(proc, "/deodxr.exe", 0, NULL);

	_KeProcessSleep(800);


	proc = _KeCreateProcess(0, "deoaud");
	_KePrint("deoaud proc id : %d \r\n", proc);
	_KeSetUID(proc, UAC_DEAMONS);
	_KeSetGID(proc, UAC_DEAMONS);
	_KeCredAddSGroup(proc, ggid_misc_world);
	_KeCredAddSGroup(proc, GROUP_AUDIO);
	_KeCredAddSGroup(proc, ggid_misc_postbox);
	_KeProcessLoadExec(proc, "/deoaud.exe", 0, NULL);


	
#elif ARCH_X64
	int proc = _KeCreateProcess(0, "deodhai");
	_KeProcessLoadExec(proc, "/deodhai.exe", 0, NULL);
#endif

	/*_KeProcessSleep(1000);
	proc = _KeCreateProcess(0, "calc");
	_KeProcessLoadExec(proc, "/calc.exe", 0, NULL);*/

	_KePrint("Setting up env variable \r\n");
	initSetupBasicEnvironmentVars();
	int sz = 0;
	while(1){
		sz = _KeReadFile(pipe, init_msg_buff, sizeof(InitRequestMsg) + 1);
		if (sz > 0) {
			_init_handle_request((InitRequestMsg*)init_msg_buff);
			memset(init_msg_buff, 0, sizeof(InitRequestMsg) + 1);
		}
		if (pid == 1)
			_KeProcessWaitForTermination(-1);
	}

	

	///* just load all the background services */
	int child = _KeCreateProcess(0, "deoaud");
	_KeProcessLoadExec(child, "/deoaud.exe", 0, NULL);

	_KeProcessSleep(1);
		
	child = _KeCreateProcess(0, "deodhai");
	_KeProcessLoadExec(child, "/deodhai.exe", 0, NULL);

	_KeProcessSleep(100);

	_KePrint("Deodhai spawned \r\n");

	/* launch the session manager directly from here */
	child = _KeCreateProcess(0, "xelnch");
	_KeProcessLoadExec(child, "/xelnch.exe", 0, NULL);

	_KeProcessSleep(100);

	child = _KeCreateProcess(0, "nmdapha");
	_KeProcessLoadExec(child, "/nmdapha.exe", 0, NULL);

	_KeProcessSleep(100);

	while (1) {
		/* from here, enter as a killer of all dead processes*/
		if (pid == 1) {
			_KeProcessWaitForTermination(-1);
		}
	}
}