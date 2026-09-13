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

#include <sys/_keproc.h>
#include <sys/_kefile.h>
#include <sys/_ketime.h>
#include <sys/_kesignal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <keycode.h>
#include <unistd.h>

char* cmdBuf;
int index;
bool _process_needed;
bool _draw_shell_curdir;
bool _spawnable_process;
int job;
bool _sig_handled = false;
char* currentDirectory;
char* lastDirectory;
int g_console_fd = -1;

void XEShellSigInterrupt(int signo) {
	/* Handle SIGINT (CTRL+C) properly */
	printf("\n[xeshell]: Received signal %d (SIGINT - CTRL+C)\r\n", signo);
	_draw_shell_curdir = true;
	if (job > 0) {
		int result = _KeSendSignal(job, signo);
		if (result == 0) {
			printf("[xeshell]: Signal sent to process %d\r\n", job);
		} else {
			printf("[xeshell]: Failed to send signal to process %d (error: %d)\r\n", job, result);
		}
		job = 0;
	}
	fflush(stdout);
}

void XEShellSignalTest(int signo) {
	printf("[xeshell]: Signal %d received (SIGINT - CTRL+C)\r\n", signo);
	fflush(stdout);
	
	if (job > 0) {
		printf("[xeshell]: Sending signal to process %d\r\n", job);
		fflush(stdout);
		_KeSendSignal(job, signo);
		job = 0;
	}
	
	printf("[xeshell]: Signal handler completed\r\n");
	fflush(stdout);
}

int timercount;
// Timer callback disabled - not available in XenevaOS
void XEShellTimerCallback(int signo) {
	// Not implemented in XenevaOS
}

/*Write the current directory string
 * for now, only root directory is
 * current directory '/'
 */
void XEShellWriteCurrentDir() {
	if (_draw_shell_curdir) {
		printf("\nXEShell %s$: ", currentDirectory ? currentDirectory : "/");
		fflush(stdout);
		_draw_shell_curdir = false;
	}
}

void XEShellSpawn(char* string) {
	if ((strlen(string)) > 0) {
		/* allocate separate memories for each strings */
		char filename[128];
		char arguments[128];
		char execname[32];
		memset(arguments, 0, sizeof(arguments));
		memset(execname, 0, sizeof(execname));

		/* _first_string_skipped is only for executable
		 * name thats why we skip early character counting */
		bool _first_string_skipped = false;
		int argcount = 0;
		int j = 0;
		char** argv = (char**)malloc(10 * sizeof(char*));
		memset(argv, 0, 10 * sizeof(char*));
		
		/* Validate input string length */
		if (strlen(string) > 127) {
			printf("\n[xeshell]: Command too long (max 127 characters)\r\n");
			return;
		}
		
		/* here we mainly prepare for the arguments to pass */
		for (int i = 0; i < strlen(string) + 1; i++) {
			if (string[i] == ' ' || string[i] == '\0') {
				if (_first_string_skipped && arguments[0] != '\0') {
					char* str = (char*)malloc(strlen(arguments) + 1);
					memset(str, 0, strlen(arguments) + 1);
					strcpy(str, arguments);
					argv[argcount] = str;
					argcount += 1;
					
					/* Check argument count limit */
					if (argcount >= 9) {
						printf("\n[xeshell]: Too many arguments (max 9)\r\n");
						/* Free allocated memory */
						for (int k = 0; k < argcount; k++) {
							free(argv[k]);
						}
						free(argv);
						return;
					}
				}
				j = 0;
				memset(arguments, 0, sizeof(arguments));

				if (!_first_string_skipped)
					_first_string_skipped = true;

				continue;
			}
			if (_first_string_skipped) {
				if (j < (int)sizeof(arguments) - 1) {
					arguments[j] = string[i];
					j++;
				}
			} else {
				if (i < (int)sizeof(execname) - 1)
					execname[i] = string[i];
			}
		}

		memset(filename, 0, sizeof(filename));
		strcpy(filename, currentDirectory);
		strcat(filename, execname);
		strcat(filename, ".exe");

		/* before spawning the process, make an entry to
		 * shell's file descriptors */
		int file = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);
		if (file == -1) {
			printf("\n[xeshell]: No command or program found: %s\r\n", filename);
			return;
		}
		
		int proc_id = _KeCreateProcess(0, string);
		if (proc_id == -1) {
			printf("\n[xeshell]: Failed to create process for: %s\r\n", string);
			_KeCloseFile(file);
			return;
		}
		
		/* Set file descriptors for the process */
		if (_KeSetFileToProcess(XENEVA_STDIN, XENEVA_STDIN, proc_id) != 0) {
			printf("\n[xeshell]: Failed to set stdin for process %d\r\n", proc_id);
			_KeCloseFile(file);
			return;
		}
		if (_KeSetFileToProcess(XENEVA_STDOUT, XENEVA_STDOUT, proc_id) != 0) {
			printf("\n[xeshell]: Failed to set stdout for process %d\r\n", proc_id);
			_KeCloseFile(file);
			return;
		}
		if (_KeSetFileToProcess(XENEVA_STDERR, XENEVA_STDERR, proc_id) != 0) {
			printf("\n[xeshell]: Failed to set stderr for process %d\r\n", proc_id);
			_KeCloseFile(file);
			return;
		}
		
		int status = _KeProcessLoadExec(proc_id, filename, argcount, argv);
		if (status != 0) {
			printf("\n[xeshell]: Failed to load executable %s (error: %d)\r\n", filename, status);
			_KeCloseFile(file);
			/* Free allocated memory */
			for (int k = 0; k < argcount; k++) {
				free(argv[k]);
			}
			free(argv);
			return;
		}

		job = proc_id;
		printf("\n[xeshell]: Started process %d for: %s\r\n", proc_id, string);
		_KeProcessWaitForTermination(proc_id);
		printf("\n[xeshell]: Process %d terminated\r\n", proc_id);
		_KeCloseFile(file);

		/* Free allocated memory */
		for (int k = 0; k < argcount; k++) {
			free(argv[k]);
		}
		free(argv);
		printf("\n");
		job = 0;
	}
}

/*
 * XEShellReadLine -- reads a line until it gets
 * and end-of-line or new line character
 */
void XEShellReadLine() {
	// Only process if we have a command to process
	// Note: We don't check _process_needed here - we always try to read
	// The flag is used in the main loop to know when to process the command
	
	// Don't clear buffer if we're in the middle of reading
	if (index == 0) {
		// Clear the command buffer only at start of new command
		memset(cmdBuf, 0, 1024);
	}
	
	// Use stdin provided by init. Init already opened /dev/console
	// and set it as XENEVA_STDIN for the shell.
	int read_fd = g_console_fd;
	
	// Read one character from console (blocks in kernel until a key arrives)
	char buf[2];
	memset(buf, 0, 2);
	int bytes_read = _KeReadFile(read_fd, buf, 1);
	if (bytes_read <= 0) {
		// No data (should be rare since kernel blocks); just return
		// and let the main loop sleep briefly before retrying.
		cmdBuf[index] = '\0';
		return;
	}
	
	char c = buf[0];
	
	if (c == '\n' || c == '\r') {
		// End of line - process the command
		printf("\n");
		fflush(stdout);
		_process_needed = true;
		return;
	}

	if (c > 0) {
		// Check bounds before writing to buffer
		if (index >= 1023) {
			printf("\n[xeshell]: Command too long (max 1023 characters)\r\n");
			fflush(stdout);
			// Clear the buffer
			memset(cmdBuf, 0, 1024);
			index = 0;
			_process_needed = true;
			return;
		}

		if (c == KEY_BACKSPACE) {
			if (index > 0) {
				printf("%c", c);
				fflush(stdout);
				cmdBuf[index--] = 0;
			}
			return;
		}
		
		if (c == KEY_SPACE) {
			printf("%c", c);
			fflush(stdout);
			cmdBuf[index] = ' ';
			index++;
			return;
		}
		
		// Only print printable characters
		if (c >= 32 && c <= 126) {
			printf("%c", c);
			fflush(stdout);
			cmdBuf[index++] = c;
		}
	}
	
	// Null-terminate the current buffer
	cmdBuf[index] = '\0';
	return;
}

/*
 * XEShellCD -- Change working Directory command
 * @param path -- path to change (supported only 
 * relative path)
 * absolute path changing not implemented
 */
void XEShellCD(char* path) {
	if (!currentDirectory || !path) {
		printf("\n[xeshell]: Invalid path or current directory\r\n");
		return;
	}

	char* newPath = NULL;
	int prevPathLen = strlen(currentDirectory);

	/* navigate back to parent directory */
	if (strcmp(path, "..") == 0) {
		if (prevPathLen == 1 && (strcmp(currentDirectory, "/") == 0)) {
			printf("\n[xeshell]: Already at root directory\r\n");
			return;
		}
		
		// Find the last '/' and truncate
		int last_slash = -1;
		for (int i = prevPathLen - 1; i >= 0; i--) {
			if (currentDirectory[i] == '/') {
				last_slash = i;
				break;
			}
		}
		
		if (last_slash != -1) {
			currentDirectory[last_slash] = '\0';
		} else {
			strcpy(currentDirectory, "/");
		}
		
		printf("\n[xeshell]: Changed to parent directory: %s\r\n", currentDirectory);
		// Update environment variable
		_XESetEnvironmentVariable("PWD", currentDirectory, 1);
		printf("[xeshell]: PWD environment variable updated to: %s\r\n", currentDirectory);
		return;
	}

	/*
	 * nothing to do, this simple switch to current directory
	 */
	if (strcmp(path, "./") == 0) {
		printf("\n[xeshell]: Already in current directory\r\n");
		return;
	}

	/*
	 * This should be the home directory that is being set on
	 * environment variable or Xeneva configured home directory
	 * but due to no implementation of environment variable
	 * passing, "~" this simple navigates back to root directory 
	 */
	if (strcmp(path, "~") == 0) {
		free(currentDirectory);
		currentDirectory = (char*)malloc(2);
		strcpy(currentDirectory, "/");
		printf("\n[xeshell]: Changed to home directory: %s\r\n", currentDirectory);
		// Update environment variable
		_XESetEnvironmentVariable("PWD", currentDirectory, 1);
		printf("[xeshell]: PWD environment variable updated to: %s\r\n", currentDirectory);
		return;
	}
	
	// Validate path length
	if (strlen(path) > 127) {
		printf("\n[xeshell]: Path too long (max 127 characters)\r\n");
		return;
	}
	
	// Check if the path exists
	char testPath[256];
	if (currentDirectory[prevPathLen - 1] == '/' && prevPathLen > 1) {
		snprintf(testPath, sizeof(testPath), "%s%s", currentDirectory, path);
	} else {
		snprintf(testPath, sizeof(testPath), "%s/%s", currentDirectory, path);
	}
	
	int testDirfd = _KeOpenDir(testPath);
	if (testDirfd == -1) {
		printf("\n[xeshell]: Directory not found: %s\r\n", testPath);
		return;
	}
	_KeCloseFile(testDirfd);
	
	// Construct new path
	if (currentDirectory[prevPathLen - 1] == '/' && prevPathLen > 1) {
		newPath = (char*)malloc(prevPathLen + strlen(path) + 1);
		strcpy(newPath, currentDirectory);
		strcat(newPath, path);
	} else {
		newPath = (char*)malloc(prevPathLen + strlen(path) + 2);
		strcpy(newPath, currentDirectory);
		strcat(newPath, "/");
		strcat(newPath, path);
	}
	
	free(currentDirectory);
	currentDirectory = newPath;
	
	// Remove trailing slash if not root
	if (strlen(currentDirectory) > 1 && currentDirectory[strlen(currentDirectory) - 1] == '/') {
		currentDirectory[strlen(currentDirectory) - 1] = '\0';
	}
	
	printf("\n[xeshell]: Changed directory to: %s\r\n", currentDirectory);

		// Update environment variable
	_XESetEnvironmentVariable("PWD", currentDirectory, 1);
	printf("[xeshell]: PWD environment variable updated to: %s\r\n", currentDirectory);
}

/*
 * XEShellLS -- list the files inside current directory
 */
void XEShellLS() {
	int dirfd = _KeOpenDir(currentDirectory);
	if (dirfd == -1) {
		printf("\n[xeshell]: Failed to open directory: %s\r\n", currentDirectory);
		return;
	}
	
	XEDirectoryEntry* dirent = (XEDirectoryEntry*)malloc(sizeof(XEDirectoryEntry));
	if (!dirent) {
		printf("\n[xeshell]: Memory allocation failed for directory entry\r\n");
		_KeCloseFile(dirfd);
		return;
	}
	memset(dirent, 0, sizeof(XEDirectoryEntry));
	
	int file_count = 0;
	printf("\n[xeshell]: Contents of %s:\r\n", currentDirectory);
	
	while (1) {
		memset(dirent->filename, 0, 32);
		int code = _KeReadDir(dirfd, dirent);
		if (code == -1) {
			break;
		}
		if (dirent->index == -1) {
			break;
		}
		
		file_count++;
		if (dirent->flags & FILE_DIRECTORY) {
			printf("\033[36m%s/\033[0m\r\n", dirent->filename);
		} else {
			printf("%s\r\n", dirent->filename);
		}
	}
	
	printf("\n[xeshell]: %d items found\r\n", file_count);
	_KeCloseFile(dirfd);
	free(dirent);
}

void XEShellPrintHelp() {
	printf("\n");
	printf("╔════════════════════════════════════════════════════════════╗\n");
	printf("║                    Xeneva Shell v1.1 Help                  ║\n");
	printf("╠════════════════════════════════════════════════════════════╣\n");
	printf("║ Command     Description                                    ║\n");
	printf("║────────────────────────────────────────────────────────────║\n");
	printf("║ cd [path]   Change current working directory               ║\n");
	printf("║ ls          List files and folders in current directory    ║\n");
	printf("║ echo [text]  Display text or write to file (echo text>file)║\n");
	printf("║ pwd         Display current working directory             ║\n");
	printf("║ clrscr      Clear entire terminal screen                   ║\n");
	printf("║ help        Show this help message                        ║\n");
	printf("║ systeminfo  Display system information                    ║\n");
	printf("║ time        Display current time                           ║\n");
	printf("║ exit        Exit the shell                                 ║\n");
	printf("╚════════════════════════════════════════════════════════════╝\n");
	printf("\n");
	printf("Examples:\n");
	printf("  cd /usr           - Change to /usr directory\n");
	printf("  cd ..             - Go to parent directory\n");
	printf("  echo Hello World   - Display Hello World\n");
	printf("  echo Test>file.txt - Write 'Test' to file.txt\n");
	printf("  ls -              - List current directory\n");
	printf("\n");
}

/*
 * XEShellPrintWorkingDirectory -- displays the current working
 * directory
 */
void XEShellPrintWorkingDirectory() {
	printf("\n[xeshell]: Current working directory: %s\r\n", currentDirectory);
}

/*
 * XEShellEcho -- outputs texts
 * @param msg -- text to output
 */
void XEShellEcho(char* msg) {
	if (!msg) {
		printf("\n[xeshell]: Missing message for echo command\r\n");
		return;
	}
	
	printf("\n");
	
	// Handle output redirection
	char* filename = strchr(msg, '>');
	if (filename) {
		filename++; // Skip '>'
		
		// Skip whitespace after '>'
		while (*filename == ' ') {
			filename++;
		}
		
		if (*filename == '\0') {
			printf("[xeshell]: Missing filename after '>'\r\n");
			return;
		}
		
		// Extract filename (until space or end of string)
		char file[128];
		int i = 0;
		while (filename[i] != ' ' && filename[i] != '\0' && i < 127) {
			file[i] = filename[i];
			i++;
		}
		file[i] = '\0';
		
		// Extract message text (before '>')
		char* msg_end = msg;
		while (*msg_end != '>' && *msg_end != '\0') {
			msg_end++;
		}
		*msg_end = '\0'; // Terminate message at '>'
		
		// Trim leading whitespace from message
		while (*msg == ' ') {
			msg++;
		}
		
		if (*msg == '\0') {
			printf("[xeshell]: No message to write to file\r\n");
			return;
		}
		
		// Write to file
		FILE* f = fopen(file, "w+");
		if (f) {
			fprintf(f, "%s", msg);
			fclose(f);
			printf("[xeshell]: Text written to file: %s\r\n", file);
		} else {
			printf("[xeshell]: Failed to write to file: %s\r\n", file);
		}
		return;
	}
	
	// Simple echo - just print the message
	printf("%s\r\n", msg);
}

/*
 *XEShellProcessLine -- processes a line by looking
 * the cmdBuf 
 */
void XEShellProcessLine() {
	if (_process_needed) {
		if (strlen(cmdBuf) == 0) {
			// Empty command, just show prompt again
			_draw_shell_curdir = true;
			goto cleanup;
		}

		if (strcmp(cmdBuf, "help") == 0) {
			XEShellPrintHelp();
			_spawnable_process = false;
		} else if (strcmp(cmdBuf, "systeminfo") == 0) {
			printf("\nXeneva Shell v1.0\n");
			printf("Copyright (C) Xeneva Private Limited 2023-2026\n");
			printf("Operating System : Xeneva v1.1 -Genuine copy\n");
			printf("Kernel Version: v1.1 \n");
#ifdef ARCH_X64
			printf("Platform: x86_64\n");
#elif ARCH_ARM64
			printf("Platform: ARMv8-A \n");
#endif
			printf("Terminal: Xeneva Terminal v1.0\n");
			printf("Window Manager: Deodhai Compositor\n");
			printf("Xeneva is made in Assam with Love \n");
			_spawnable_process = false;
		} else if (strcmp(cmdBuf, "clrscr") == 0) {
			printf("\033[2J\033[H"); // Clear screen and move cursor to home
			printf("[xeshell]: Screen cleared\r\n");
			_spawnable_process = false;
		} else if (strcmp(cmdBuf, "time") == 0) {
			printf("\nCurrent time is : ");
			XETime time;
			memset(&time, 0, sizeof(XETime));
			if (_KeGetCurrentTime(&time) == 0) {
				uint8_t hour = time.hour;
				char* pmam = "AM";
				if (hour > 12) {
					hour -= 12;
					pmam = "PM";
				} else if (hour == 0) {
					hour = 12;
				} else if (hour == 12) {
					pmam = "PM";
				}
				printf("%d:%02d %s\r\n", hour, time.minute, pmam);
			} else {
				printf("[xeshell]: Failed to get current time\r\n");
			}
			_spawnable_process = false;
		} else if (strcmp(cmdBuf, "ls") == 0) {
			XEShellLS();
			_spawnable_process = false;
		} else if (strncmp(cmdBuf, "cd ", 3) == 0 || strcmp(cmdBuf, "cd") == 0) {
			char* path = cmdBuf + 2;
			if (*path == ' ') {
				while (*path == ' ')
					path++;
			}
			if (*path == '\0') {
				// No path provided, show current directory
				XEShellPrintWorkingDirectory();
			} else {
				XEShellCD(path);
			}
			printf("\n");
			_spawnable_process = false;
		} else if (strcmp(cmdBuf, "pwd") == 0) {
			XEShellPrintWorkingDirectory();
			_spawnable_process = false;
		} else if (strncmp(cmdBuf, "echo ", 5) == 0 || strcmp(cmdBuf, "echo") == 0) {
			char* msg = cmdBuf + 4;
			if (*msg == ' ') {
				while (*msg == ' ')
					msg++;
			}
			XEShellEcho(msg);
			_spawnable_process = false;
		} else if (strcmp(cmdBuf, "exit") == 0) {
			printf("[xeshell]: Exiting shell...\r\n");
			_KeProcessExit();
		} else if (_spawnable_process) {
			XEShellSpawn(cmdBuf);
		}

cleanup:
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
int main(int argc, char* arv[]) {
#ifdef ARCH_ARM64
	printf("Xeneva Shell v1.1 (arm64 build)\n");
#elif ARCH_X64
	printf("Xeneva Shell v1.1 (x86_64 build) \n");
#endif

	printf("Copyright (C) Xeneva Private Limited \n");
	fflush(stdout);
	
	// Initialize command buffer
	cmdBuf = (char*)malloc(1024);
	if (!cmdBuf) {
		printf("[xeshell]: Failed to allocate command buffer\r\n");
		return 1;
	}
	memset(cmdBuf, 0, 1024);
	
	// Initialize current directory
	currentDirectory = (char*)malloc(2);
	if (!currentDirectory) {
		printf("[xeshell]: Failed to allocate current directory buffer\r\n");
		free(cmdBuf);
		return 1;
	}
	strcpy(currentDirectory, "/");
	
	// Initialize shell state
	_process_needed = false;
	_spawnable_process = true;
	_draw_shell_curdir = true;
	_sig_handled = false;
	job = 0;
	index = 0;
	timercount = 0;
	g_console_fd = XENEVA_STDIN;
	
	// Set environment variable
	_XESetEnvironmentVariable("PWD", currentDirectory, 0);
	printf("[xeshell]: PWD environment variable initialized to: %s\r\n", currentDirectory);
	
	// Set up signal handling
	if (_KeSetSignal(SIGINT, XEShellSignalTest) != 0) {
		printf("[xeshell]: Warning: Failed to set up signal handler\r\n");
	}
	// Timer functionality disabled - not available in XenevaOS
	
	printf("[xeshell]: Shell initialized successfully\r\n");
	fflush(stdout);
	
	// Timer test code disabled - not available in XenevaOS
#if 0
	// Unix/Linux timer functions not available in XenevaOS
	// This code is commented out
#endif
	
	while (1) {
		XEShellWriteCurrentDir();
		XEShellReadLine();
		if (_process_needed) {
			XEShellProcessLine();
			_process_needed = false;
			index = 0; // Reset index for next command
		}
		_KeProcessSleep(10); // Increased sleep for better responsiveness
	}
}