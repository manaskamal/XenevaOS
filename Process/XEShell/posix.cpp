/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\utsname.h>
#include <sys\_keproc.h>
#include <sys\_ketime.h>

typedef int (*posix_fn)(int argc, char** argv);

typedef struct {
	const char* name;
	posix_fn fn;
} PosixCommand;

int exec_uname(const char* option) {
	struct utsname name;
	memset(&name, 0, sizeof(utsname));
	if (uname(&name)) {
		printf("failed to get system information \r\n");
		return 1;
	}
	if (strcmp(option, "-s") == 0) {
		printf("%s\n", name.sysname);
	} else if (strcmp(option, "-n") == 0) {
		printf("%s\n", name.nodename);
	} else if (strcmp(option, "-r") == 0) {
		printf("%s\n", name.release);
	} else if (strcmp(option, "-v") == 0) {
		printf("%s\n", name.version);
	} else if (strcmp(option, "-m") == 0) {
		printf("%s\r\n", name.machine);
	} else if (strcmp(option, "-p") == 0) {
		printf("%s\r\n", name.machine);
	} else if (strcmp(option, "-i") == 0) {
		printf("unknown\r\n");
	} else if (strcmp(option, "-o") == 0) {
		printf("XenevaOS\r\n");
	} else if (strcmp(option, "-a") == 0) {
		printf("system name : %s \r\n", name.sysname);
		printf("nodename : %s \r\n", name.nodename);
		printf("release: %s \r\n", name.release);
		printf("version: %s \r\n", name.version);
		printf("machine: %s \r\n", name.machine);
		printf("processor: %s \r\n", name.machine);
		printf("build name : jajabori\r\n");
	}
	return 0;
}

/**
 * @brief posix_uname -- posix uname command implementation
 */
int posix_uname(int argc, char** argv) {
	if (argc == 1)
		return exec_uname("-s");
	for (int i = 1; i < argc; i++)
		exec_uname(argv[i]);
	return 0;
}

int posix_who(int argc, char** argv) {
	bool header = false, count = false;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] != '\0') {
			for (char* f = argv[i] + 1; *f; f++) {
				switch (*f) {
				case 'H':
					header = true;
					break;
				case 'q':
					count = true;
					break;
				case 'm':
					break;
				default:
					printf("who: invalid option -- '%c'\r\n", *f);
					return 1;
				}
			}
		} else if (strcmp(argv[i], "am") == 0 && i + 1 < argc && strcmp(argv[i + 1], "i") == 0) {
			i++;
		} else {
			printf("who: extra operand '%s'\r\n", argv[i]);
			return 1;
		}
	}

	const char* user = "root";
	const char* tty = "term0";

	if (count) {
		printf("%s\r\n# users=1\r\n", user);
		return 0;
	}
	if (header) {
		printf("not implemented\r\n");
	}
	return 0;
}

int posix_clear(int argc, char** argv) {
	printf("\033[2J\033[H");
	fflush(stdout);
	return 0;
}

extern volatile int g_interrupted;

/**
 * @brief posix_sleep -- posix sleep implementation
 */
int posix_sleep(int argc, char** argv) {
	if (argc != 2) {
		printf("sleep: usage: sleep SECONDS\r\n");
		return 1;
	}

	const char* p = argv[1];
	uint64_t ms = 0, frac_ms = 0, scale = 100;
	bool digits = false;

	for (; *p >= '0' && *p <= '9'; p++) {
		ms = ms * 10 + (*p - '0');
		digits = true;
		if (ms > 86400) {
			ms = 86400;
		}
	}
	ms *= 1000;
	if (*p == '.') {
		for (p++; *p >= '0' && *p <= '9'; p++) {
			if (scale) {
				frac_ms += (*p - '0') * scale;
				scale /= 10;
			}
			digits = true;
		}
	}
	if (!digits || *p != '\0') {
		printf("sleep: invalid time interval '%s' \r\n", argv[1]);
		return 1;
	}

	ms += frac_ms;

	g_interrupted = 0;
	uint64_t end = _KeGetCurrentMS() + ms;
	while (!g_interrupted) {
		uint64_t now = _KeGetCurrentMS();
		if (now >= end)
			break;
		uint64_t left = end - now;
		_KeProcessSleep(left > 100 ? 100 : (int)left);
	}
}

#define POSIX_MAX_ARGS 16

/**
 * posix command handler
 */
static const PosixCommand posix_table[] = {
	{"uname", posix_uname},
	{"who", posix_who},
	{"clear", posix_clear},
	{"sleep", posix_sleep},
};

/**
 * @brief_term_handle_posix -- handle posix commands
 * @param cmd -- pointer to shell's command buffer
 */
int _term_handle_posix(char* cmd) {
	char line[256];
	char* argv[POSIX_MAX_ARGS];
	int argc = 0;

	strncpy(line, cmd, sizeof(line) - 1);
	line[sizeof(line) - 1] = '\0';

	char* p = line;
	while (*p && argc < POSIX_MAX_ARGS - 1) {
		while (*p == ' ')
			*p++ = '\0';
		if (!*p)
			break;
		argv[argc++] = p;
		while (*p && *p != ' ')
			p++;
	}
	argv[argc] = NULL;
	if (argc == 0)
		return -1;

	for (size_t i = 0; i < sizeof(posix_table) / sizeof(posix_table[0]); i++) {
		if (strcmp(argv[0], posix_table[i].name) == 0)
			return posix_table[i].fn(argc, argv);
	}
	return -1;
}