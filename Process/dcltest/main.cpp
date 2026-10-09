/*
 * Process/dcltest/main.cpp -- userspace end-to-end test for the DCL ports.
 *
 * DCL/mem_test.c proves the chain from inside the kernel, but it cannot reach
 * the half that actually matters to users: a real process calling
 * _KeOpenFile() -> fileserv -> devfs -> mainline file_operations, carrying
 * its own credentials. That distinction is not academic -- the DCL nodes are
 * created uid0/gid-world (AURORA_GID_MISC_WORLD), init hands XEShell that
 * group as a supplementary group, and a bare normal-user process is refused.
 * So run this from the shell / Terminal:
 *
 *     /dcltest.exe
 *
 * Every result is emitted to stdout (so you can read it in the shell) and to
 * /dev/kmsg, whose write path is mainline-style fops ending in UARTDebugOut.
 * That makes the same transcript appear in the QEMU serial log, so CI can
 * assert on it without a display:
 *
 *     grep '\[dcltest\]' run.log
 *
 * Exit code is 0 when every check passed, otherwise the number of failures.
 */

#include <_xeneva.h>
#include <stdio.h>
#include <string.h>
#include <sys/_kefile.h>

static int g_ok;
static int g_bad;
static int g_kmsg = -1;

/* emit -- one line to both the caller's console and the kernel log. */
static void emit(const char* line) {
	printf("%s", line);
	if (g_kmsg < 0)
		g_kmsg = _KeOpenFile((char*)"/dev/kmsg", FILE_OPEN_WRITE);
	if (g_kmsg >= 0)
		_KeWriteFile(g_kmsg, (void*)line, strlen(line));
}

/* report -- assert got == want. */
static void report(const char* what, int got, int want) {
	char line[168];
	if (got == want) {
		g_ok++;
		sprintf(line, "[dcltest] PASS %s (%d)\n", what, got);
	} else {
		g_bad++;
		sprintf(line, "[dcltest] FAIL %s got=%d want=%d\n", what, got, want);
	}
	emit(line);
}

/* note -- record something without counting it as a check. */
static void note(const char* what, int value) {
	char line[168];
	sprintf(line, "[dcltest] info %s (%d)\n", what, value);
	emit(line);
}

/* opendev -- open a node through the real syscall path; the open itself is a
 * check, because that is where mainline's memory_open() per-minor fops swap
 * happens (fileserv calls node->open at Serv/fileserv.c:127). */
static int opendev(const char* path, int mode) {
	char line[168];
	int fd = _KeOpenFile((char*)path, mode);
	if (fd < 0) {
		g_bad++;
		sprintf(line, "[dcltest] FAIL open %s fd=%d\n", path, fd);
	} else {
		g_ok++;
		sprintf(line, "[dcltest] PASS open %s fd=%d\n", path, fd);
	}
	emit(line);
	return fd;
}

int main(int argc, char* argv[]) {
	unsigned char a[64], b[64];
	int fd, n, i, differs;

	(void)argc;
	(void)argv;
	memset(a, 0, sizeof a);
	memset(b, 0, sizeof b);
	emit("[dcltest] --- DCL userspace test start ---\n");

	/* /dev/null: writes are consumed, reads are EOF */
	fd = opendev("/dev/null", FILE_OPEN_WRITE);
	if (fd >= 0) {
		memset(a, 0xAA, sizeof a);
		report("null write", (int)_KeWriteFile(fd, a, 8), 8);
		_KeCloseFile(fd);
	}
	fd = opendev("/dev/null", FILE_OPEN_READ_ONLY);
	if (fd >= 0) {
		memset(a, 0xAA, sizeof a);
		report("null read (EOF)", (int)_KeReadFile(fd, a, 8), 0);
		_KeCloseFile(fd);
	}

	/* /dev/zero: writes consumed, reads yield zero-filled chunks */
	fd = opendev("/dev/zero", FILE_OPEN_READ_ONLY);
	if (fd >= 0) {
		memset(a, 0xAA, sizeof a);
		n = (int)_KeReadFile(fd, a, 32);
		report("zero read", n, 32);
		differs = 1;
		for (i = 0; i < n && i < (int)sizeof a; i++)
			if (a[i])
				differs = 0;
		report("zero bytes are 0", differs, 1);
		_KeCloseFile(fd);
	}
	fd = opendev("/dev/zero", FILE_OPEN_WRITE);
	if (fd >= 0) {
		memset(a, 0xAA, 8);
		report("zero write", (int)_KeWriteFile(fd, a, 8), 8);
		_KeCloseFile(fd);
	}

	/* /dev/full: reads EOF, writes report no space as a short count */
	fd = opendev("/dev/full", FILE_OPEN_WRITE);
	if (fd >= 0) {
		memset(a, 0xAA, 8);
		report("full write (ENOSPC)", (int)_KeWriteFile(fd, a, 8), 0);
		_KeCloseFile(fd);
	}
	fd = opendev("/dev/full", FILE_OPEN_READ_ONLY);
	if (fd >= 0) {
		report("full read (EOF)", (int)_KeReadFile(fd, a, 8), 0);
		_KeCloseFile(fd);
	}

	/* /dev/urandom: entropy from the hardware RNG, not a constant */
	fd = opendev("/dev/urandom", FILE_OPEN_READ_ONLY);
	if (fd >= 0) {
		memset(a, 0, sizeof a);
		memset(b, 0, sizeof b);
		n = (int)_KeReadFile(fd, a, 16);
		report("urandom read", n, 16);
		_KeReadFile(fd, b, 16);
		differs = 0;
		for (i = 0; i < 16; i++)
			if (a[i] != b[i])
				differs = 1;
		report("urandom samples differ", differs, 1);
		_KeCloseFile(fd);
	}

	/* /dev/random: same backing, blocking-free here because it is hwrng-backed */
	fd = opendev("/dev/random", FILE_OPEN_READ_ONLY);
	if (fd >= 0) {
		memset(a, 0, sizeof a);
		report("random read", (int)_KeReadFile(fd, a, 16), 16);
		_KeCloseFile(fd);
	}

	/* /dev/kmsg: a write reaches the kernel log, a read is EOF (never blocks,
	 * which is why this test writes rather than draining the log). */
	{
		const char* msg = "[dcltest] kmsg write from userspace reached UART\n";
		fd = opendev("/dev/kmsg", FILE_OPEN_WRITE);
		if (fd >= 0) {
			report("kmsg write", (int)_KeWriteFile(fd, (void*)msg, strlen(msg)),
				   (int)strlen(msg));
			_KeCloseFile(fd);
		}
		fd = opendev("/dev/kmsg", FILE_OPEN_READ_ONLY);
		if (fd >= 0) {
			report("kmsg read (EOF)", (int)_KeReadFile(fd, a, 8), 0);
			_KeCloseFile(fd);
		}
	}

	/* /dev/mem: open only. open_port() walks capable() ->
	 * security_locked_down() -> iomem_get_mapping() -> memory_open(), which is
	 * mainline's gate path; reading arbitrary physical addresses is left to an
	 * interactive session, exactly as DCL/mem_test.c does it. */
	fd = opendev("/dev/mem", FILE_OPEN_READ_ONLY);
	if (fd >= 0) {
		note("mem gate path reached (read left to a human)", 1);
		_KeCloseFile(fd);
	}

	/* /dev/hwrng: Xeneva's own node, and the entropy source behind random */
	fd = opendev("/dev/hwrng", FILE_OPEN_READ_ONLY);
	if (fd >= 0) {
		memset(a, 0, sizeof a);
		n = (int)_KeReadFile(fd, a, 16);
		report("hwrng read", n, 16);
		_KeCloseFile(fd);
	}

	/* /dev/dcl: DCL's status node (open + read only) */
	fd = opendev("/dev/dcl", FILE_OPEN_READ_ONLY);
	if (fd >= 0) {
		n = (int)_KeReadFile(fd, a, sizeof a);
		report("dcl status readable", n >= 0, 1);
		note("dcl status bytes", n);
		_KeCloseFile(fd);
	}

	/* /dev/clipboard: a write replaces the contents, a read reports them
	 * without consuming -- pasting twice has to paste twice. */
	fd = opendev("/dev/clipboard", FILE_OPEN_WRITE);
	if (fd >= 0) {
		report("clipboard write", (int)_KeWriteFile(fd, (void*)"hello, dcl", 10),
			   10);
		_KeCloseFile(fd);
	}
	fd = opendev("/dev/clipboard", FILE_OPEN_READ_ONLY);
	if (fd >= 0) {
		char cb[64];
		memset(cb, 0, sizeof cb);
		report("clipboard read", (int)_KeReadFile(fd, cb, sizeof cb), 10);
		report("clipboard bytes", (int)memcmp(cb, "hello, dcl", 10), 0);
		report("clipboard read again", (int)_KeReadFile(fd, cb, sizeof cb), 10);
		_KeCloseFile(fd);
	}

	{
		char line[168];
		sprintf(line, "[dcltest] SUMMARY %d ok, %d failed\n", g_ok, g_bad);
		emit(line);
	}
	if (g_kmsg >= 0)
		_KeCloseFile(g_kmsg);
	return g_bad;
}
