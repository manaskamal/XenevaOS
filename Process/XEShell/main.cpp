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


void XEShellSigInterrupt(int signo) {
	printf("Shell Interrupted \n");
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
		char filename[32];
		memset(filename, 0, 32);
		strcpy(filename, "/");
		strcpy(filename + 1, string);
		strcpy(filename + strlen(string), ".exe");
		/* before spawning the process, make an entry to
		 * shell's file descriptors */
		int file = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);
		if (file == -1){
			printf("\nNo command or program found with name %s \n", string);
			return;
		}
		int proc_id = _KeCreateProcess(0, string);
		_KeSetFileToProcess(XENEVA_STDIN, XENEVA_STDIN, proc_id);
		_KeSetFileToProcess(XENEVA_STDOUT, XENEVA_STDOUT, proc_id);
		_KeSetFileToProcess(XENEVA_STDERR, XENEVA_STDERR, proc_id);
		int status = _KeProcessLoadExec(proc_id, filename, 0, 0);
		_KeProcessWaitForTermination(proc_id);
		_KeCloseFile(file);
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
			index++;
			return;
		}
		cmdBuf[index++] = c;
	}
	cmdBuf[index] = '\0';
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
			_KeGetCurrentTime(&time);
			printf("\%d:", (time.hour-12));
			printf("%d\n", time.minute);
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
	_KeSetSignal(SIGINT, XEShellSigInterrupt);
	cmdBuf = (char*)malloc(1024);
	memset(cmdBuf, 0, 1024);
	_process_needed = true;
	_spawnable_process = true;
	index = 0;
	while (1){
		XEShellWriteCurrentDir();
		XEShellReadLine();
		XEShellProcessLine();
		_KeProcessSleep(1000000000000);
	}
}