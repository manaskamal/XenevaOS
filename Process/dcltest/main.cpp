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

/* ── stage 2: KMS properties, the libdrm two-step ─────────────────────────
 *
 * OBJ_GETPROPERTIES for an object's property ids -- the count pass first,
 * then the arrays -- and GETPROPERTY for each name.  The struct layouts are
 * the uapi's, spelled like every other ioctl struct in this file; the ioctl
 * numbers carry the sizes (32 and 64). */
struct drm_mode_obj_getprops_arg {
	unsigned long long props_ptr, prop_values_ptr;
	unsigned int count_props, obj_id, obj_type;
};
struct drm_mode_getprop_arg {
	unsigned long long values_ptr, enum_blob_ptr;
	unsigned int prop_id, flags;
	char name[32];
	unsigned int count_values, count_enum_blobs;
};

#define DRM_IOCTL_MODE_OBJ_GETPROPERTIES 0xC02064B9u	/* _IOWR('d', 0xB9, 32) */
#define DRM_IOCTL_MODE_GETPROPERTY 0xC04064AAu		/* _IOWR('d', 0xAA, 64) */

/* DRM_MODE_OBJECT_* -- obj_type for kms_prop_id(), from drm_mode_object.h */
#define DRM_MODE_OBJECT_CONNECTOR 0xC0C0C0C0u
#define DRM_MODE_OBJECT_CRTC 0xCCCCCCCCu
#define DRM_MODE_OBJECT_PLANE 0xEEEEEEEEu

/* kms_prop_id -- the id of the property named `want` on obj_id of obj_type,
 * or 0 (never a property id) when the object or name is not there. */
static unsigned int kms_prop_id(int fd, unsigned int obj_id,
				unsigned int obj_type, const char* want) {
	static unsigned int ids[64];
	static unsigned long long vals[64];
	struct drm_mode_obj_getprops_arg op;
	unsigned int n, i;

	memset(&op, 0, sizeof op);
	op.obj_id = obj_id;
	op.obj_type = obj_type;
	if (_KeFileIoControl(fd, DRM_IOCTL_MODE_OBJ_GETPROPERTIES, &op) < 0)
		return 0;
	n = op.count_props;
	if (n == 0 || n > 64)
		return 0;

	memset(&op, 0, sizeof op);
	op.obj_id = obj_id;
	op.obj_type = obj_type;
	op.props_ptr = (unsigned long long)(size_t)ids;
	op.prop_values_ptr = (unsigned long long)(size_t)vals;
	op.count_props = n;
	if (_KeFileIoControl(fd, DRM_IOCTL_MODE_OBJ_GETPROPERTIES, &op) < 0)
		return 0;

	for (i = 0; i < n; i++) {
		struct drm_mode_getprop_arg gp;
		memset(&gp, 0, sizeof gp);
		gp.prop_id = ids[i];
		if (_KeFileIoControl(fd, DRM_IOCTL_MODE_GETPROPERTY, &gp) < 0)
			continue;
		if (strcmp(gp.name, want) == 0)
			return ids[i];
	}
	return 0;
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

	/* /dev/dri/card0 -- the vkms DRM device. Two ioctls, and both go the
	 * whole way: devfs -> fileserv -> the DCL cdev bridge -> mainline's
	 * file_operations->unlocked_ioctl -> drm_ioctl() -> vkms.
	 *
	 * The ioctl numbers are spelled out because this file cannot include
	 * kernel headers. Both are _IOWR('d', nr, type): direction 3 in the
	 * top two bits, sizeof(type) in bits 16..29, 'd' = 0x64 below that and
	 * nr in the low byte. struct drm_version is 64 bytes (three ints, then
	 * three length/pointer pairs) and struct drm_get_cap is 16, which is
	 * where 0x40 and 0x10 come from.
	 *
	 * The lengths are size_t -- 8 bytes here -- because the kernel's are
	 * __kernel_size_t, which BaseHdr/linux/types.h defines as size_t.
	 * `unsigned long` would be 4 bytes (this app and the kernel are both
	 * aarch64-unknown-windows, which is LLP64), which shifts the three
	 * pointers to offsets 16, 32 and 48; the kernel then reads `desc` from
	 * past the end of this struct and copy_to_user()s the driver string
	 * into stack garbage.
	 */
	{
		struct drm_version_arg {
			int major, minor, patchlevel;
			size_t name_len;
			char* name;
			size_t date_len;
			char* date;
			size_t desc_len;
			char* desc;
		} ver;
		struct drm_get_cap_arg {
			unsigned long long capability;
			unsigned long long value;
		} cap;
		char vname[32], vdesc[64];
		int vfd = opendev("/dev/dri/card0",
				  FILE_OPEN_READ_ONLY | FILE_OPEN_WRITE);

		if (vfd >= 0) {
			memset(&ver, 0, sizeof ver);
			memset(vname, 0, sizeof vname);
			memset(vdesc, 0, sizeof vdesc);
			ver.name = vname;
			ver.name_len = sizeof vname;
			ver.desc = vdesc;
			ver.desc_len = sizeof vdesc;
			report("drm VERSION", _KeFileIoControl(vfd, 0xC0406400, &ver), 0);
			report("drm version major", ver.major, 1);
			report("drm name length", (int)ver.name_len, 4);
			report("drm name is vkms", (int)memcmp(vname, "vkms", 4), 0);
			report("drm desc length", (int)ver.desc_len,
				   (int)strlen("Virtual Kernel Mode Setting"));

			memset(&cap, 0, sizeof cap);
			cap.capability = 0x1;	/* DRM_CAP_DUMB_BUFFER */
			report("drm GET_CAP", _KeFileIoControl(vfd, 0xC010640C, &cap), 0);
			report("drm dumb-buffer cap", (int)cap.value, 1);

			/* What the kernel gates the probes below on: only
			 * overlay planes are listed by GETPLANERESOURCES
			 * until DRM_CLIENT_CAP_UNIVERSAL_PLANES (2), and
			 * the writeback connector is hidden from
			 * GETRESOURCES until DRM_CLIENT_CAP_WRITEBACK_
			 * CONNECTORS (5), which itself requires ATOMIC (3).
			 * _IOW('d', 0x0D, struct drm_set_client_cap): two
			 * __u64s, so 0x10 in the size bits.
			 */
			struct drm_set_client_cap_arg {
				unsigned long long capability, value;
			} scc;

			scc.capability = 2;
			scc.value = 1;
			report("drm kms universal planes cap",
				   _KeFileIoControl(vfd, 0x4010640D, &scc), 0);
			scc.capability = 3;
			report("drm kms atomic cap",
				   _KeFileIoControl(vfd, 0x4010640D, &scc), 0);
			scc.capability = 5;
			report("drm kms writeback connectors cap",
				   _KeFileIoControl(vfd, 0x4010640D, &scc), 0);

			/* The KMS half of the same device.  vkms's default
			 * configuration -- this tree's module params: cursor
			 * on, writeback on, overlay off -- is one CRTC, a
			 * primary and a cursor plane, two encoders (the
			 * config's VIRTUAL one plus the writeback pair's)
			 * and two connectors: the VIRTUAL one (type 15) and
			 * the writeback connector (type 18), sharing the
			 * CRTC.  The caps above are what makes all of them
			 * visible.  GETRESOURCES counts CRTCs, encoders and
			 * connectors; planes are enumerated only by
			 * GETPLANERESOURCES.
			 *
			 * Same _IOWR('d', nr, type) spelling as above:
			 * card_res 0xA0/0x40, mode_crtc 0xA1/0x68 (36 header
			 * bytes plus the 68-byte modeinfo), get_encoder
			 * 0xA6/0x14, get_connector 0xA7/0x50, plane_res
			 * 0xB5/0x10, plane 0xB6/0x20.
			 *
			 * The _ptr fields take arrays of __u32 object ids;
			 * the count fields are both input buffer size and
			 * output true count -- the libdrm two-pass pattern.
			 * GETCONNECTOR's count pass (count_modes 0) is what
			 * makes vkms build the mode list.  The arrays are
			 * static: 128 modes are 8704 bytes and this stack is
			 * small.
			 */
			struct drm_mode_card_res_arg {
				unsigned long long fb_id_ptr, crtc_id_ptr;
				unsigned long long connector_id_ptr;
				unsigned long long encoder_id_ptr;
				unsigned int count_fbs, count_crtcs;
				unsigned int count_connectors, count_encoders;
				unsigned int min_width, max_width;
				unsigned int min_height, max_height;
			} res;
			struct drm_mode_modeinfo_arg {
				unsigned int clock;
				unsigned short hdisplay, hsync_start, hsync_end;
				unsigned short htotal, hskew;
				unsigned short vdisplay, vsync_start, vsync_end;
				unsigned short vtotal, vscan;
				unsigned int vrefresh, flags, type;
				char name[32];
			};
			struct drm_mode_crtc_arg {
				unsigned long long set_connectors_ptr;
				unsigned int count_connectors;
				unsigned int crtc_id, fb_id, x, y;
				unsigned int gamma_size, mode_valid;
				struct drm_mode_modeinfo_arg mode;
			} crtc;
			struct drm_mode_get_connector_arg {
				unsigned long long encoders_ptr, modes_ptr;
				unsigned long long props_ptr, prop_values_ptr;
				unsigned int count_modes, count_props;
				unsigned int count_encoders;
				unsigned int encoder_id, connector_id;
				unsigned int connector_type, connector_type_id;
				unsigned int connection, mm_width, mm_height;
				unsigned int subpixel, pad;
			} conn;
			struct drm_mode_get_encoder_arg {
				unsigned int encoder_id, encoder_type;
				unsigned int crtc_id, possible_crtcs;
				unsigned int possible_clones;
			} enc;
			struct drm_mode_get_plane_res_arg {
				unsigned long long plane_id_ptr;
				unsigned int count_planes;
			} pres;
			struct drm_mode_get_plane_arg {
				unsigned int plane_id, crtc_id, fb_id;
				unsigned int possible_crtcs, gamma_size;
				unsigned int count_format_types;
				unsigned long long format_type_ptr;
			} pl;
			static unsigned int crtc_ids[4], conn_ids[4];
			static unsigned int enc_ids[4], conn_encs[4];
			static unsigned int plane_ids[4], formats[64];
			static struct drm_mode_modeinfo_arg modes[128];
			int virt = -1, prim_xrgb = 0;
			unsigned int virt_type = 0, wb_type = 0;

			memset(&res, 0, sizeof res);
			report("drm kms GETRESOURCES",
				   _KeFileIoControl(vfd, 0xC04064A0, &res), 0);
			report("drm kms crtcs", (int)res.count_crtcs, 1);
			report("drm kms connectors",
				   (int)res.count_connectors, 2);
			report("drm kms encoders", (int)res.count_encoders, 2);
			report("drm kms fbs", (int)res.count_fbs, 0);
			report("drm kms bounds 10..8192",
				   (int)(res.min_width == 10 &&
					 res.min_height == 10 &&
					 res.max_width == 8192 &&
					 res.max_height == 8192), 1);

			res.crtc_id_ptr = (unsigned long long)(size_t)crtc_ids;
			res.count_crtcs = 4;
			res.connector_id_ptr = (unsigned long long)(size_t)conn_ids;
			res.count_connectors = 4;
			res.encoder_id_ptr = (unsigned long long)(size_t)enc_ids;
			res.count_encoders = 4;
			report("drm kms GETRESOURCES (ids)",
				   _KeFileIoControl(vfd, 0xC04064A0, &res), 0);
			report("drm kms crtc id",
				   (int)(crtc_ids[0] != 0), 1);
			report("drm kms connector ids",
				   (int)(conn_ids[0] != 0 && conn_ids[1] != 0 &&
					 conn_ids[0] != conn_ids[1]), 1);
			report("drm kms encoder id",
				   (int)(enc_ids[0] != 0), 1);

			memset(&pres, 0, sizeof pres);
			report("drm kms GETPLANERESOURCES",
				   _KeFileIoControl(vfd, 0xC01064B5, &pres), 0);
			report("drm kms planes", (int)pres.count_planes, 2);
			pres.plane_id_ptr = (unsigned long long)(size_t)plane_ids;
			pres.count_planes = 4;
			report("drm kms GETPLANERESOURCES (ids)",
				   _KeFileIoControl(vfd, 0xC01064B5, &pres), 0);
			report("drm kms plane ids",
				   (int)(plane_ids[0] != 0 && plane_ids[1] != 0 &&
					 plane_ids[0] != plane_ids[1]), 1);

			for (int k = 0; k < 2; k++) {
				memset(&pl, 0, sizeof pl);
				memset(formats, 0, sizeof formats);
				pl.plane_id = plane_ids[k];
				report("drm kms GETPLANE",
					   _KeFileIoControl(vfd, 0xC02064B6, &pl), 0);
				report("drm kms plane formats",
					   (int)(pl.count_format_types > 0), 1);
				report("drm kms plane possible_crtcs",
					   (int)(pl.possible_crtcs & 1), 1);
				pl.format_type_ptr =
					(unsigned long long)(size_t)formats;
				pl.count_format_types = 64;
				_KeFileIoControl(vfd, 0xC02064B6, &pl);
				if (k == 0)
					for (unsigned int f = 0; f < 64; f++)
						/* XRGB8888, as fourcc_code
						 * packs it: 'X','R','2','4' */
						if (formats[f] == 0x34325258)
							prim_xrgb = 1;
			}
			report("drm kms primary takes XRGB8888", prim_xrgb, 1);

			memset(&crtc, 0, sizeof crtc);
			crtc.crtc_id = crtc_ids[0];
			report("drm kms GETCRTC",
				   _KeFileIoControl(vfd, 0xC06864A1, &crtc), 0);
			report("drm kms crtc id round-trip",
				   (int)(crtc.crtc_id == crtc_ids[0]), 1);
			report("drm kms crtc unprogrammed",
				   (int)crtc.mode_valid, 0);

			memset(&enc, 0, sizeof enc);
			enc.encoder_id = enc_ids[0];
			report("drm kms GETENCODER",
				   _KeFileIoControl(vfd, 0xC01464A6, &enc), 0);
			report("drm kms encoder is virtual",
				   (int)enc.encoder_type, 5);
			report("drm kms encoder possible_crtcs",
				   (int)(enc.possible_crtcs & 1), 1);

			memset(&enc, 0, sizeof enc);
			enc.encoder_id = enc_ids[1];
			report("drm kms GETENCODER (second)",
				   _KeFileIoControl(vfd, 0xC01464A6, &enc), 0);

			for (int k = 0; k < 2; k++) {
				memset(&conn, 0, sizeof conn);
				conn.connector_id = conn_ids[k];
				report("drm kms GETCONNECTOR",
					   _KeFileIoControl(vfd, 0xC05064A7, &conn),
					   0);
				/* the split is by type: both connectors
				 * report the DMT mode list, so the mode
				 * count does not tell them apart */
				if (conn.connector_type == 15) {
					virt = k;
					virt_type = conn.connector_type;
				} else {
					wb_type = conn.connector_type;
				}
			}
			report("drm kms virtual connector found",
				   (int)(virt >= 0), 1);
			report("drm kms virtual connector type",
				   (int)virt_type, 15);
			report("drm kms writeback connector type",
				   (int)wb_type, 18);

			if (virt >= 0) {
				memset(&conn, 0, sizeof conn);
				memset(modes, 0, sizeof modes);
				conn.connector_id = conn_ids[virt];
				conn.modes_ptr =
					(unsigned long long)(size_t)modes;
				conn.count_modes = 128;
				conn.encoders_ptr =
					(unsigned long long)(size_t)conn_encs;
				conn.count_encoders = 4;
				report("drm kms GETCONNECTOR (modes)",
					   _KeFileIoControl(vfd, 0xC05064A7, &conn),
					   0);
				report("drm kms connector connected",
					   (int)conn.connection, 1);
				report("drm kms connector modes fit",
					   (int)(conn.count_modes > 0 &&
						 conn.count_modes <= 128), 1);
				int pref = 0;
				static struct drm_mode_modeinfo_arg mode1024;
				for (unsigned int k = 0;
					 k < conn.count_modes && k < 128; k++)
					if (modes[k].hdisplay == 1024 &&
					    modes[k].vdisplay == 768) {
						pref = 1;
						memcpy(&mode1024, &modes[k],
						       sizeof mode1024);
					}
				report("drm kms preferred mode 1024x768",
					   pref, 1);
				/* encoder_id is the legacy field and
				 * reads 0 here -- nothing has programmed
				 * a CRTC yet.  The connector's encoder is
				 * its possible-encoder list, just copied
				 * into conn_encs. */
				report("drm kms connector encoder id",
					   (int)(conn_encs[0] == enc_ids[0] ||
						 conn_encs[0] == enc_ids[1]), 1);
				report("drm kms connector props",
					   (int)(conn.count_props > 0), 1);

				/* ---- stage 2: dumb buffer -> ADDFB2 -> atomic ---- */
				struct drm_mode_create_dumb_arg {
					unsigned int height, width, bpp, flags;
					unsigned int handle, pitch;
					unsigned long long size;
				} dumb;
				struct drm_mode_map_dumb_arg {
					unsigned int handle, pad;
					unsigned long long offset;
				} map;
				struct drm_mode_destroy_dumb_arg {
					unsigned int handle, pad;
				} dd;
				struct drm_mode_fb_cmd2_arg {
					unsigned int fb_id, width, height;
					unsigned int pixel_format, flags;
					unsigned int handles[4], pitches[4], offsets[4];
					unsigned long long modifier[4];
				} fb;
				struct drm_mode_create_blob_arg {
					unsigned long long data;
					unsigned int length, blob_id;
				} blob;
				struct drm_mode_atomic_arg {
					unsigned int flags, count_objs;
					unsigned long long objs_ptr, count_props_ptr;
					unsigned long long props_ptr, prop_values_ptr;
					unsigned long long reserved, user_data;
				} at;
				unsigned int conn_crtc_id, crtc_mode_id, crtc_active;
				unsigned int plane_fb_id, plane_crtc_id;
				unsigned int src_x, src_y, src_w, src_h;
				unsigned int crtc_x, crtc_y, crtc_w, crtc_h;
				static unsigned int a_objs[3], a_counts[3];
				static unsigned int a_props[13];
				static unsigned long long a_vals[13];
				int rc;

				/* CREATE_DUMB, 1024x768 XRGB8888: the uapi
				 * promises pitch 4096 and a PAGE_SIZE-aligned
				 * size, 3 MiB. */
				memset(&dumb, 0, sizeof dumb);
				dumb.width = 1024;
				dumb.height = 768;
				dumb.bpp = 32;
				rc = _KeFileIoControl(vfd, 0xC02064B2, &dumb);
				report("drm kms CREATE_DUMB", rc, 0);
				report("drm kms dumb handle",
					   (int)(dumb.handle != 0), 1);
				report("drm kms dumb pitch", (int)dumb.pitch, 4096);
				report("drm kms dumb size",
					   (int)(dumb.size == 3145728ULL), 1);

				/* MAP_DUMB: mainline's handler produces the
				 * fake mmap offset the uapi promises, and DCL's
				 * ioctl tail (drm_stubs.c) replaces it with a
				 * user address this app dereferences directly --
				 * there is no mmap on this target to hand the
				 * offset to. */
				memset(&map, 0, sizeof map);
				map.handle = dumb.handle;
				rc = _KeFileIoControl(vfd, 0xC01064B3, &map);
				report("drm kms MAP_DUMB", rc, 0);
				report("drm kms MAP_DUMB offset",
					   (int)(map.offset != 0 &&
						 (map.offset & 0xFFFull) == 0), 1);

				/* both ends of the frame through the mapping,
				 * written and read back: the pages are really
				 * there and really the object's. */
				{
					volatile unsigned int* pix =
						(volatile unsigned int*)(size_t)map.offset;
					pix[0] = 0x00FF00FFu;
					pix[1024 * 768 - 1] = 0x000000FFu;
					report("drm kms dumb mapping write/read",
						   (int)(pix[0] == 0x00FF00FFu &&
							 pix[1024 * 768 - 1] ==
								 0x000000FFu), 1);
				}

				/* ADDFB2 over the handle, XRGB8888 linear. */
				memset(&fb, 0, sizeof fb);
				fb.width = 1024;
				fb.height = 768;
				fb.pixel_format = 0x34325258; /* 'X','R','2','4' */
				fb.handles[0] = dumb.handle;
				fb.pitches[0] = dumb.pitch;
				rc = _KeFileIoControl(vfd, 0xC06864B8, &fb);
				report("drm kms ADDFB2", rc, 0);
				report("drm kms fb id", (int)(fb.fb_id != 0), 1);

				/* the property ids the atomic request needs. */
				conn_crtc_id = kms_prop_id(vfd, conn_ids[virt],
							   DRM_MODE_OBJECT_CONNECTOR, "CRTC_ID");
				crtc_mode_id = kms_prop_id(vfd, crtc_ids[0],
							   DRM_MODE_OBJECT_CRTC, "MODE_ID");
				crtc_active = kms_prop_id(vfd, crtc_ids[0],
							  DRM_MODE_OBJECT_CRTC, "ACTIVE");
				plane_fb_id = kms_prop_id(vfd, plane_ids[0],
							  DRM_MODE_OBJECT_PLANE, "FB_ID");
				plane_crtc_id = kms_prop_id(vfd, plane_ids[0],
							    DRM_MODE_OBJECT_PLANE, "CRTC_ID");
				src_x = kms_prop_id(vfd, plane_ids[0],
						    DRM_MODE_OBJECT_PLANE, "SRC_X");
				src_y = kms_prop_id(vfd, plane_ids[0],
						    DRM_MODE_OBJECT_PLANE, "SRC_Y");
				src_w = kms_prop_id(vfd, plane_ids[0],
						    DRM_MODE_OBJECT_PLANE, "SRC_W");
				src_h = kms_prop_id(vfd, plane_ids[0],
						    DRM_MODE_OBJECT_PLANE, "SRC_H");
				crtc_x = kms_prop_id(vfd, plane_ids[0],
						     DRM_MODE_OBJECT_PLANE, "CRTC_X");
				crtc_y = kms_prop_id(vfd, plane_ids[0],
						     DRM_MODE_OBJECT_PLANE, "CRTC_Y");
				crtc_w = kms_prop_id(vfd, plane_ids[0],
						     DRM_MODE_OBJECT_PLANE, "CRTC_W");
				crtc_h = kms_prop_id(vfd, plane_ids[0],
						     DRM_MODE_OBJECT_PLANE, "CRTC_H");
				report("drm kms atomic property ids",
					   (int)(conn_crtc_id && crtc_mode_id &&
						 crtc_active && plane_fb_id &&
						 plane_crtc_id && src_x && src_y &&
						 src_w && src_h && crtc_x && crtc_y &&
						 crtc_w && crtc_h), 1);

				/* the mode as a blob: crtc MODE_ID takes one,
				 * and the one mode the connector offers that
				 * the mm bounds and this app agree on. */
				memset(&blob, 0, sizeof blob);
				blob.data = (unsigned long long)(size_t)&mode1024;
				blob.length = sizeof mode1024;
				rc = _KeFileIoControl(vfd, 0xC01064BD, &blob);
				report("drm kms CREATEPROPBLOB", rc, 0);
				report("drm kms blob id",
					   (int)(blob.blob_id != 0), 1);

				/* the atomic request: connector -> crtc, crtc
				 * mode + active, plane carrying the fb over the
				 * full 1024x768 with a 16.16 source rectangle. */
				a_objs[0] = conn_ids[virt];
				a_counts[0] = 1;
				a_objs[1] = crtc_ids[0];
				a_counts[1] = 2;
				a_objs[2] = plane_ids[0];
				a_counts[2] = 10;
				{
					unsigned int p = 0;
					a_props[p] = conn_crtc_id;
					a_vals[p++] = crtc_ids[0];
					a_props[p] = crtc_mode_id;
					a_vals[p++] = blob.blob_id;
					a_props[p] = crtc_active;
					a_vals[p++] = 1;
					a_props[p] = plane_fb_id;
					a_vals[p++] = fb.fb_id;
					a_props[p] = plane_crtc_id;
					a_vals[p++] = crtc_ids[0];
					a_props[p] = src_x;
					a_vals[p++] = 0;
					a_props[p] = src_y;
					a_vals[p++] = 0;
					a_props[p] = src_w;
					a_vals[p++] = 1024ULL << 16;
					a_props[p] = src_h;
					a_vals[p++] = 768ULL << 16;
					a_props[p] = crtc_x;
					a_vals[p++] = 0;
					a_props[p] = crtc_y;
					a_vals[p++] = 0;
					a_props[p] = crtc_w;
					a_vals[p++] = 1024;
					a_props[p] = crtc_h;
					a_vals[p++] = 768;
				}
				memset(&at, 0, sizeof at);
				at.count_objs = 3;
				at.objs_ptr = (unsigned long long)(size_t)a_objs;
				at.count_props_ptr =
					(unsigned long long)(size_t)a_counts;
				at.props_ptr = (unsigned long long)(size_t)a_props;
				at.prop_values_ptr =
					(unsigned long long)(size_t)a_vals;

				/* TEST_ONLY|ALLOW_MODESET first: the whole
				 * request is vetted before anything is applied.
				 * TEST_ONLY is 0x100 -- 0x200 is NONBLOCK, which
				 * still lands the state for real. */
				at.flags = 0x100 | 0x400;
				report("drm kms atomic TEST_ONLY",
					   _KeFileIoControl(vfd, 0xC03864BC, &at), 0);

				/* and for real.  ALLOW_MODESET covers first-time
				 * mode programming; no PAGE_FLIP_EVENT, this
				 * app has no event queue. */
				at.flags = 0x400;
				report("drm kms atomic commit",
					   _KeFileIoControl(vfd, 0xC03864BC, &at), 0);

				/* GETCRTC now reports the programmed mode:
				 * mode_valid is the driver's enable flag. */
				memset(&crtc, 0, sizeof crtc);
				crtc.crtc_id = crtc_ids[0];
				report("drm kms GETCRTC (programmed)",
					   _KeFileIoControl(vfd, 0xC06864A1, &crtc), 0);
				report("drm kms crtc programmed",
					   (int)crtc.mode_valid, 1);
				report("drm kms crtc mode 1024x768",
					   (int)(crtc.mode.hdisplay == 1024 &&
						 crtc.mode.vdisplay == 768), 1);

				/* disable: plane off, crtc inactive with its
				 * mode cleared, connector detached -- the same
				 * three objects again.  MODE_ID=0 is what turns
				 * enable off; a crtc left "enabled" with no
				 * connectors attached is rejected by the helper
				 * (enabled/connectors mismatch). */
				a_counts[0] = 1;
				a_props[0] = conn_crtc_id;
				a_vals[0] = 0;
				a_counts[1] = 2;
				a_props[1] = crtc_active;
				a_vals[1] = 0;
				a_props[2] = crtc_mode_id;
				a_vals[2] = 0;
				a_counts[2] = 2;
				a_props[3] = plane_fb_id;
				a_vals[3] = 0;
				a_props[4] = plane_crtc_id;
				a_vals[4] = 0;
				at.flags = 0x400;
				report("drm kms atomic disable",
					   _KeFileIoControl(vfd, 0xC03864BC, &at), 0);
				memset(&crtc, 0, sizeof crtc);
				crtc.crtc_id = crtc_ids[0];
				_KeFileIoControl(vfd, 0xC06864A1, &crtc);
				report("drm kms crtc unprogrammed again",
					   (int)crtc.mode_valid, 0);

				/* RMFB then DESTROY_DUMB, then the object is
				 * gone: a second MAP_DUMB fails the lookup. */
				{
					unsigned int rmfb_id = fb.fb_id;
					report("drm kms RMFB",
						   _KeFileIoControl(vfd, 0xC00464AF,
								    &rmfb_id), 0);
				}
				memset(&dd, 0, sizeof dd);
				dd.handle = dumb.handle;
				report("drm kms DESTROY_DUMB",
					   _KeFileIoControl(vfd, 0xC00864B4, &dd), 0);
				memset(&map, 0, sizeof map);
				map.handle = dumb.handle;
				report("drm kms MAP_DUMB after destroy fails",
					   (int)(_KeFileIoControl(vfd, 0xC01064B3,
								  &map) != 0), 1);
			}
			_KeCloseFile(vfd);
		}
	}

	/* privileged device nodes belong to the daemons group; an ordinary process (this
	 * one has world only) must be refused even when it asks for a write-mode open,
	 * which used to skip the permission check entirely */
	{
		static const char* priv[] = {"/dev/hwdev", "/dev/drvctl", "/dev/sndsrv"};
		for (int k = 0; k < 3; k++) {
			char what[64];
			sprintf(what, "%s refuses an ordinary process (write open)", priv[k]);
			int pfd = _KeOpenFile((char*)priv[k], FILE_OPEN_WRITE);
			report(what, pfd < 0, 1);
			if (pfd >= 0)
				_KeCloseFile(pfd);
		}
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
