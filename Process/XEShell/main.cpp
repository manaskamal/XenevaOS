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

#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys\_ketime.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <keycode.h>

char *cmdBuf;
int index;
bool _process_needed;
bool _draw_shell_curdir;
bool _spawnable_process;
int job;
bool _sig_handled = false;

void XEShellSigInterrupt(int signo) {
	/* signal is little buggy so, so it won't
	 * work for now
	 */
	printf("Signal is buggy, won't work properly");
	_draw_shell_curdir = true;
	if (job > 0) {
		_KeSendSignal(job, signo);
		job = 0;
	}
	/* here is the bug, we need a system call to 
	 * handle it properly
	 */
	_KeProcessSleep(10);
}

/*Write the current directory string
 * for now, only root directory is
 * current directory '/'
 */
void XEShellWriteCurrentDir() {
	if (_draw_shell_curdir){
		printf("\n");
		printf("\033[32m\033[40mXEShell /$:\033[37m\033[40m");
		_draw_shell_curdir = false;
	}
}

void XEShellSpawn(char* string) {
	if ((strlen(string)-1) > 0) {

		/* allocate separate memories for each strings */
		char filename[32];
		char arguments[32];
		char execname[32];
		memset(arguments, 0, 32);
		memset(execname, 0, 32);

		/* _first_string_skipped is only for executable
		 * name thats why we skip early character counting */
		bool _first_string_skipped = false;
		int argcount = 0;
		int index = 0;
		int j = 0;
		int totalCharacterCount = strlen(string);
		char** argv = (char**)malloc(10);
		memset(argv, 0, 10);
		/* here we mainly prepare for the arguments to pass */
		for (int i = 0; i < strlen(string); i++){
			if (string[i] == ' ' || string[i] == '\0'){
				index = i;
				_KePrint("space -> %d  \r\n", _first_string_skipped);

				if (_first_string_skipped) {
					char* str = (char*)malloc(strlen(arguments));
					memset(str, 0, strlen(arguments));
					strcpy(str, arguments);
					argv[argcount] = str;
					_KePrint("[XESHELL]: Arguments %s %d\r\n", argv[argcount], _KeGetProcessID());
					argcount += 1;
					j = 0;
					memset(arguments, 0, 32);
				}

				if (!_first_string_skipped)
					_first_string_skipped = true;
				
				continue;
			}
			if (_first_string_skipped){
				arguments[j] = string[i];
				j++;
			}

			if (!_first_string_skipped){
				execname[i] = string[i];
			}
		}
	
		memset(filename, 0, 32);
		strcpy(filename, "/");
		strcpy(filename + 1, execname);
		strcpy(filename + strlen(execname), ".exe");
	
		/* before spawning the process, make an entry to
		 * shell's file descriptors */
		int file = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);
		if (file == -1){
			printf("\n No command or program found \n");
			return;
		}
		int proc_id = _KeCreateProcess(0, string);
		_KeSetFileToProcess(XENEVA_STDIN, XENEVA_STDIN, proc_id);
		_KeSetFileToProcess(XENEVA_STDOUT, XENEVA_STDOUT, proc_id);
		_KeSetFileToProcess(XENEVA_STDERR, XENEVA_STDERR, proc_id);
		int status = _KeProcessLoadExec(proc_id, filename, argcount, argv);


		job = proc_id;
		_KeProcessWaitForTermination(proc_id);
		_KeCloseFile(file);

		while (argcount) {
			argv++;
			argcount--;
		}
		job = 0;
	}
}

/*
 * XEShellReadLine -- reads a line until it gets
 * and end-of-line or new line character
 */ 
void XEShellReadLine() {
	char c = getchar();
	if (c == '\n') {
		_process_needed = true;
		return;
	}

	if (c > 0) {
		if (index == 1024) {
			_process_needed = true;
			return;
		}
		printf("%c", c);
		if (c == KEY_BACKSPACE){
			if (index > 0){
				cmdBuf[index--] = 0;
			}
			if (index <= 0)
				_draw_shell_curdir = true;
			return;
		}
		if (c == KEY_SPACE){
			cmdBuf[index] = ' ';
			index++;
			return;
		}
		cmdBuf[index++] = c;
	}
	cmdBuf[index] = '\0';
}

void XEShellLS() {
	int dirfd = _KeOpenDir("/");
	XEDirectoryEntry* dirent = (XEDirectoryEntry*)malloc(sizeof(XEDirectoryEntry));
	memset(dirent, 0, sizeof(XEDirectoryEntry));

	while (1) {
		if (dirent->index == -1)
			break;
		int code = _KeReadDir(dirfd, dirent);
		if (code != -1) {
			if (dirent->flags & FILE_DIRECTORY){
				printf("\033[36m %s\n", dirent->filename);
			}
			else{
				printf("%s \n", dirent->filename);
			}
		}
		memset(dirent->filename, 0, 32);
	}
}

/*
 *XEShellProcessLine -- processes a line by looking
 * the cmdBuf 
 */
void XEShellProcessLine() {
	if (_process_needed) {
		if (strcmp(cmdBuf, "help") == 0)  {
			printf("\nWelcome to Xeneva shell v1.0 \n");
			printf("clrscr -- Clear entire terminal screen \n");
			printf("help -- prints all command with their descriptions\n");
			printf("systeminfo -- prints about message \n");
			printf("time -- displays the current time \n");
			_spawnable_process = false;
		}

		if (strcmp(cmdBuf, "systeminfo") == 0) {
			printf("\nXeneva Shell v1.0\n");
			printf("Copyright (C) Manas Kamal Choudhury 2023\n");
			printf("Operating System : Xeneva v1.0 -Genuine copy\n");
			printf("Kernel Version: v1.0 \n");
			printf("Platform: x86_64\n");
			printf("Terminal: Xeneva Terminal v1.0\n");
			printf("Window Manager: Deodhai Compositor\n");
			printf("Xeneva is made in Assam with Love \n");
			_spawnable_process = false;
		}

		if (strcmp(cmdBuf, "clrscr") == 0) {
			printf("\033[2J");
			_spawnable_process = false;
		}

		if (strcmp(cmdBuf, "time") == 0) {
			printf("\nCurrent time is : ");
			XETime time;
			memset(&time, 0, sizeof(XETime));
			_KeGetCurrentTime(&time);
			uint8_t hour = time.hour;
			char* pmam = "AM";
			if (hour > 12) {
				hour -= 12;
				pmam = "PM";
			}
			printf("\%d:", hour);
			printf("%d ", time.minute);
			printf("%s\n", pmam);
			_spawnable_process = false;
		} 

		if (strcmp(cmdBuf, "ls") == 0){
			XEShellLS();
			_spawnable_process = false;
		}

		if (_spawnable_process)
			XEShellSpawn(cmdBuf);

		memset(cmdBuf, 0, 1024);
		index = 0;
		_process_needed = false;
		_draw_shell_curdir = true;
		_spawnable_process = true;
	}
}

/*
* main -- terminal emulator
*/
int main(int argc, char* arv[]){
	printf("Xeneva Shell v1.0 \n");
	printf("Copyright (C) Manas Kamal Choudhury 2023\n");
	printf("\033[44m^[30mHello World\n");
	printf("\033[42m^[37mXeneva is an operating system\n");
	printf("^[42m^[37mMade in North-East India,Assam\n");
	printf("SignalHandler address -> %x \n", XEShellSigInterrupt);
	_KeSetSignal(SIGINT, XEShellSigInterrupt);
	cmdBuf = (char*)malloc(1024);
	memset(cmdBuf, 0, 1024);
	_process_needed = true;
	_spawnable_process = true;
	_sig_handled = false;
	job = 0;
	index = 0;
	while (1){
		XEShellWriteCurrentDir();
		XEShellReadLine();
		XEShellProcessLine();
		_KeProcessSleep(8);
	}
}