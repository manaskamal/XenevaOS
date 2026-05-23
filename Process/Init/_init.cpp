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


void initSetupBasicEnvironmentVars() {
	setenv("HOME", "/", 1);
	setenv("USER", "xeuser", 1);
	setenv("USERNAME", "xeuser", 1);
	setenv("SHELL", "XenevaTerminal", 1);
	setenv("LANG", "en_US.UTF-8", 1); //Set it to English(United States), UTF-8
	setenv("OSNAME", "XenevaOS", 1);
}



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
	init_basic_gid_to_dev();
	/** TODO: add IPC system to track real system progress and animate the logo accordingly **/
	_KeProcessSleep(500);

	int ggid_misc_world = _KeGetGlobalGroupID(AURORA_GID_MISC_WORLD);
	int ggid_misc_postbox = _KeGetGlobalGroupID(AURORA_GID_IPC_POSTBOX);


#ifdef ARCH_ARM64
	int proc = _KeCreateProcess(0, "netmngr");
	_KeSetUID(proc, UAC_DEAMONS);
	_KeSetGID(proc, UAC_DEAMONS);
	_KeCredAddSGroup(proc, ggid_misc_world);
	_KeProcessLoadExec(proc, "/netmngr.exe\0", 0, NULL);

	/** actually, design should be like that, each process after
	 * finish its initialization, it should send a signal to 
	 * init, so that it can continue next proccesses spawning
	 */
	_KeProcessSleep(500);


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
	while(1){
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