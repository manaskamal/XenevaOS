#include "qemu_egl_capture.h"

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <gbm.h>

#include <gio/gio.h>
#include <gio/gunixfdlist.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

static const char* k_listener_xml =
	"<node>"
	"  <interface name='org.qemu.Display1.Listener'>"
	"    <method name='Scanout'>"
	"      <arg type='u' name='width' direction='in'/>"
	"      <arg type='u' name='height' direction='in'/>"
	"      <arg type='u' name='stride' direction='in'/>"
	"      <arg type='u' name='pixman_format' direction='in'/>"
	"      <arg type='ay' name='data' direction='in'/>"
	"    </method>"
	"    <method name='Update'>"
	"      <arg type='i' name='x' direction='in'/>"
	"      <arg type='i' name='y' direction='in'/>"
	"      <arg type='i' name='width' direction='in'/>"
	"      <arg type='i' name='height' direction='in'/>"
	"      <arg type='u' name='stride' direction='in'/>"
	"      <arg type='u' name='pixman_format' direction='in'/>"
	"      <arg type='ay' name='data' direction='in'/>"
	"    </method>"
	"    <method name='ScanoutDMABUF'>"
	"      <arg type='h' name='dmabuf' direction='in'/>"
	"      <arg type='u' name='width' direction='in'/>"
	"      <arg type='u' name='height' direction='in'/>"
	"      <arg type='u' name='stride' direction='in'/>"
	"      <arg type='u' name='fourcc' direction='in'/>"
	"      <arg type='t' name='modifier' direction='in'/>"
	"      <arg type='b' name='y0_top' direction='in'/>"
	"    </method>"
	"    <method name='UpdateDMABUF'>"
	"      <arg type='i' name='x' direction='in'/>"
	"      <arg type='i' name='y' direction='in'/>"
	"      <arg type='i' name='width' direction='in'/>"
	"      <arg type='i' name='height' direction='in'/>"
	"    </method>"
	"    <method name='Disable'/>"
	"    <method name='MouseSet'>"
	"      <arg type='i' name='x' direction='in'/>"
	"      <arg type='i' name='y' direction='in'/>"
	"      <arg type='i' name='on' direction='in'/>"
	"    </method>"
	"    <method name='CursorDefine'>"
	"      <arg type='i' name='width' direction='in'/>"
	"      <arg type='i' name='height' direction='in'/>"
	"      <arg type='i' name='hot_x' direction='in'/>"
	"      <arg type='i' name='hot_y' direction='in'/>"
	"      <arg type='ay' name='data' direction='in'/>"
	"    </method>"
	"  </interface>"
	"</node>";

static std::mutex g_mu;
static std::vector<uint8_t> g_rgba;
static int g_w, g_h;
static bool g_have;
/* Reverse dbus connection to QEMU (we are the client) plus the console we
 * registered on (scanout path). Injection goes over the session bus instead:
 * QEMU only hosts our Listener object on the reverse connection, while the
 * Mouse/Keyboard interfaces live on the session bus under its well-known
 * name. */
static GDBusConnection* g_p2p;
static std::string g_console_path;
static GDBusConnection* g_sbus;
static std::string g_qemu_name;
/* Last injected pointer position (dedup: don't spam QEMU). */
static std::mutex g_mu_abs;
static uint32_t g_last_mx = 0xFFFFFFFFu, g_last_my = 0xFFFFFFFFu;
/* Last DMABUF scanout, kept (dup'd fd) so UpdateDMABUF damage notifies can
 * re-read the same buffer. QEMU only sends a fresh ScanoutDMABUF when the
 * buffer itself changes. */
static int g_dmabuf_fd = -1;
static uint32_t g_dmabuf_w, g_dmabuf_h, g_dmabuf_stride, g_dmabuf_fourcc;
static uint64_t g_dmabuf_mod;
static bool g_dmabuf_valid;

static EGLDisplay g_egl = EGL_NO_DISPLAY;
static EGLContext g_ctx = EGL_NO_CONTEXT;
static struct gbm_device* g_gbm;
static int g_drm_fd = -1;
static PFNEGLCREATEIMAGEKHRPROC p_eglCreateImageKHR;
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC p_glEGLImageTargetTexture2DOES;

static bool egl_init() {
	if (g_egl != EGL_NO_DISPLAY)
		return true;
	g_drm_fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
	if (g_drm_fd < 0)
		g_drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (g_drm_fd < 0) {
		std::fprintf(stderr, "qemu-egl: no DRM render node\n");
		return false;
	}
	g_gbm = gbm_create_device(g_drm_fd);
	if (!g_gbm) {
		std::fprintf(stderr, "qemu-egl: gbm_create_device failed\n");
		return false;
	}
	g_egl = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, g_gbm, nullptr);
	if (g_egl == EGL_NO_DISPLAY)
		g_egl = eglGetDisplay((EGLNativeDisplayType)g_gbm);
	if (g_egl == EGL_NO_DISPLAY || !eglInitialize(g_egl, nullptr, nullptr)) {
		std::fprintf(stderr, "qemu-egl: egl display/init failed (err=0x%x)\n", eglGetError());
		return false;
	}
	eglBindAPI(EGL_OPENGL_ES_API);
	EGLint cfg_attr[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE,
						 EGL_PBUFFER_BIT, EGL_NONE};
	EGLConfig cfg;
	EGLint n = 0;
	if (!eglChooseConfig(g_egl, cfg_attr, &cfg, 1, &n) || n < 1) {
		std::fprintf(stderr, "qemu-egl: eglChooseConfig failed (err=0x%x)\n", eglGetError());
		return false;
	}
	EGLint ctx_attr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
	g_ctx = eglCreateContext(g_egl, cfg, EGL_NO_CONTEXT, ctx_attr);
	if (g_ctx == EGL_NO_CONTEXT) {
		std::fprintf(stderr, "qemu-egl: eglCreateContext failed (err=0x%x)\n", eglGetError());
		return false;
	}
	if (!eglMakeCurrent(g_egl, EGL_NO_SURFACE, EGL_NO_SURFACE, g_ctx)) {
		/* Surfaceless contexts need EGL_KHR_surfaceless_context; fall back
		 * to a 1x1 pbuffer which works everywhere. err 0x3002 here means
		 * EGL_BAD_ACCESS from the surfaceless bind. */
		std::fprintf(stderr, "qemu-egl: surfaceless bind failed (err=0x%x), trying pbuffer\n",
					 eglGetError());
		EGLint pb_attr[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
		EGLSurface pb = eglCreatePbufferSurface(g_egl, cfg, pb_attr);
		if (pb == EGL_NO_SURFACE) {
			std::fprintf(stderr, "qemu-egl: eglCreatePbufferSurface failed (err=0x%x)\n",
						 eglGetError());
			return false;
		}
		if (!eglMakeCurrent(g_egl, pb, pb, g_ctx)) {
			std::fprintf(stderr, "qemu-egl: eglMakeCurrent(pbuffer) failed (err=0x%x)\n",
						 eglGetError());
			return false;
		}
	}
	p_eglCreateImageKHR =
		(PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	p_glEGLImageTargetTexture2DOES =
		(PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	std::fprintf(stderr, "qemu-egl: GBM/EGL ready (dmabuf import %s)\n",
				 (p_eglCreateImageKHR && p_glEGLImageTargetTexture2DOES) ? "yes" : "no");
	return true;
}

static void store_bgra(const uint8_t* src, int w, int h, int stride) {
	std::lock_guard<std::mutex> lock(g_mu);
	g_w = w;
	g_h = h;
	g_rgba.resize((size_t)w * (size_t)h * 4);
	for (int y = 0; y < h; y++) {
		const uint8_t* row = src + y * stride;
		uint8_t* dst = g_rgba.data() + (size_t)y * w * 4;
		for (int x = 0; x < w; x++) {
			dst[x * 4 + 0] = row[x * 4 + 2];
			dst[x * 4 + 1] = row[x * 4 + 1];
			dst[x * 4 + 2] = row[x * 4 + 0];
			dst[x * 4 + 3] = 255;
		}
	}
	g_have = true;
}

static bool import_dmabuf(int fd, uint32_t w, uint32_t h, uint32_t stride, uint32_t fourcc,
						  uint64_t modifier) {
	if (!egl_init() || !p_eglCreateImageKHR || !p_glEGLImageTargetTexture2DOES)
		return false;
	if (!eglMakeCurrent(g_egl, EGL_NO_SURFACE, EGL_NO_SURFACE, g_ctx))
		return false;
	EGLint attrs[20];
	int i = 0;
	attrs[i++] = EGL_WIDTH;
	attrs[i++] = (EGLint)w;
	attrs[i++] = EGL_HEIGHT;
	attrs[i++] = (EGLint)h;
	attrs[i++] = EGL_LINUX_DRM_FOURCC_EXT;
	attrs[i++] = (EGLint)fourcc;
	attrs[i++] = EGL_DMA_BUF_PLANE0_FD_EXT;
	attrs[i++] = fd;
	attrs[i++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
	attrs[i++] = 0;
	attrs[i++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
	attrs[i++] = (EGLint)stride;
	if (modifier) {
		attrs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
		attrs[i++] = (EGLint)(modifier & 0xffffffffu);
		attrs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
		attrs[i++] = (EGLint)(modifier >> 32);
	}
	attrs[i++] = EGL_NONE;
	EGLImage img =
		p_eglCreateImageKHR(g_egl, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, attrs);
	if (img == EGL_NO_IMAGE_KHR) {
		std::fprintf(stderr, "qemu-egl: eglCreateImageKHR failed (fourcc=0x%x)\n", fourcc);
		return false;
	}
	GLuint tex = 0, fbo = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	p_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, img);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	std::vector<uint8_t> tmp((size_t)w * h * 4);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, (GLsizei)w, (GLsizei)h, GL_RGBA, GL_UNSIGNED_BYTE, tmp.data());
	/* glReadPixels returns bottom-row-first; the capture buffer convention
	 * here is top-row-first (same as the X11 and Scanout paths), so flip. */
	{
		std::lock_guard<std::mutex> lock(g_mu);
		g_w = (int)w;
		g_h = (int)h;
		g_rgba.resize((size_t)w * h * 4);
		for (uint32_t y = 0; y < h; y++) {
			std::memcpy(g_rgba.data() + (size_t)y * w * 4,
						tmp.data() + (size_t)(h - 1 - y) * w * 4, (size_t)w * 4);
		}
		g_have = true;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &tex);
	eglDestroyImage(g_egl, img);
	return true;
}

static void method_call(GDBusConnection*, const gchar*, const gchar*, const gchar*,
						const gchar* method, GVariant* params, GDBusMethodInvocation* inv,
						gpointer) {
	if (!std::strcmp(method, "Scanout")) {
		guint32 w, h, stride, fmt;
		GVariant* data_v = nullptr;
		g_variant_get(params, "(uuuu@ay)", &w, &h, &stride, &fmt, &data_v);
		gsize n = 0;
		const guint8* data = (const guint8*)g_variant_get_fixed_array(data_v, &n, 1);
		{
			int64_t need = (int64_t)(h > 0 ? h - 1 : 0) * stride + (int64_t)w * 4;
			if ((int64_t)n < need) {
				std::fprintf(stderr, "qemu-egl: Scanout w=%u h=%u stride=%u fmt=0x%x datalen=%zu need=%lld%s\n",
							 w, h, stride, fmt, n, (long long)need,
							 (int64_t)n < need ? " SHORT-DATA" : "");
				std::fflush(stderr);
			}
		}
		if (data && w && h)
			store_bgra(data, (int)w, (int)h, (int)stride);
		g_variant_unref(data_v);
		g_dbus_method_invocation_return_value(inv, nullptr);
		return;
	}
	if (!std::strcmp(method, "Update")) {
		gint32 x, y, w, h;
		guint32 stride, fmt;
		GVariant* data_v = nullptr;
		g_variant_get(params, "(iiiiuu@ay)", &x, &y, &w, &h, &stride, &fmt, &data_v);
		gsize n = 0;
		const guint8* data = (const guint8*)g_variant_get_fixed_array(data_v, &n, 1);
		/* Reject malformed geometry before it can index outside the scanout.
		 * The payload must cover (h-1)*stride + w*4 bytes. --axiss */
		{
			int64_t need = (int64_t)(h > 0 ? h - 1 : 0) * stride + (int64_t)w * 4;
			if (x < 0 || y < 0 || w <= 0 || h <= 0 || (int64_t)n < need) {
				std::fprintf(stderr,
							 "qemu-egl: Update x=%d y=%d w=%d h=%d stride=%u fmt=0x%x "
							 "datalen=%zu need=%lld%s\n",
							 x, y, w, h, stride, fmt, n, (long long)need,
							 (int64_t)n < need ? " SHORT-DATA" : "");
				std::fflush(stderr);
			}
		}
		if (data && w > 0 && h > 0) {
			std::lock_guard<std::mutex> lock(g_mu);
			if ((int)(x + w) <= g_w && (int)(y + h) <= g_h && !g_rgba.empty()) {
				for (int row = 0; row < h; row++) {
					const uint8_t* s = data + row * stride;
					uint8_t* d = g_rgba.data() + ((size_t)(y + row) * g_w + x) * 4;
					for (int col = 0; col < w; col++) {
						d[col * 4 + 0] = s[col * 4 + 2];
						d[col * 4 + 1] = s[col * 4 + 1];
						d[col * 4 + 2] = s[col * 4 + 0];
						d[col * 4 + 3] = 255;
					}
				}
			}
		}
		g_variant_unref(data_v);
		g_dbus_method_invocation_return_value(inv, nullptr);
		return;
	}
	if (!std::strcmp(method, "ScanoutDMABUF")) {
		GDBusMessage* msg = g_dbus_method_invocation_get_message(inv);
		GUnixFDList* fds = g_dbus_message_get_unix_fd_list(msg);
		gint32 hidx = 0;
		guint32 w = 0, ht = 0, stride = 0, fourcc = 0;
		guint64 mod = 0;
		gboolean y0 = TRUE;
		g_variant_get(params, "(huuuutb)", &hidx, &w, &ht, &stride, &fourcc, &mod, &y0);
		gint fd = -1;
		if (fds)
			fd = g_unix_fd_list_get(fds, hidx, nullptr);
		{
			static bool logged = false;
			static uint32_t last_fourcc = 0;
			if (!logged || fourcc != last_fourcc) {
				std::fprintf(stderr, "qemu-egl: scanout %ux%u stride=%u fourcc=0x%x mod=0x%llx\n",
							 w, ht, stride, fourcc, (unsigned long long)mod);
				std::fflush(stderr);
				logged = true;
				last_fourcc = fourcc;
			}
		}
		if (fd >= 0) {
			import_dmabuf(fd, w, ht, stride, fourcc, mod);
			/* Keep our own reference so later UpdateDMABUF notifies
			 * (same buffer, new damage) can re-read it. */
			int kept = dup(fd);
			if (kept >= 0) {
				std::lock_guard<std::mutex> lock(g_mu);
				if (g_dmabuf_fd >= 0)
					close(g_dmabuf_fd);
				g_dmabuf_fd = kept;
				g_dmabuf_w = w;
				g_dmabuf_h = ht;
				g_dmabuf_stride = stride;
				g_dmabuf_fourcc = fourcc;
				g_dmabuf_mod = mod;
				g_dmabuf_valid = true;
			}
			close(fd);
		}
		g_dbus_method_invocation_return_value(inv, nullptr);
		return;
	}
	if (!std::strcmp(method, "UpdateDMABUF")) {
		gint32 x = 0, y = 0, w = 0, h = 0;
		g_variant_get(params, "(iiii)", &x, &y, &w, &h);
		(void)x;
		(void)y;
		(void)w;
		(void)h;
		int fd = -1;
		uint32_t cw = 0, ch = 0, cs = 0, cf = 0;
		uint64_t cm = 0;
		{
			std::lock_guard<std::mutex> lock(g_mu);
			if (g_dmabuf_valid && g_dmabuf_fd >= 0) {
				fd = dup(g_dmabuf_fd);
				cw = g_dmabuf_w;
				ch = g_dmabuf_h;
				cs = g_dmabuf_stride;
				cf = g_dmabuf_fourcc;
				cm = g_dmabuf_mod;
			}
		}
		if (fd >= 0) {
			import_dmabuf(fd, cw, ch, cs, cf, cm);
			close(fd);
		}
		g_dbus_method_invocation_return_value(inv, nullptr);
		return;
	}
	g_dbus_method_invocation_return_value(inv, nullptr);
}

static const GDBusInterfaceVTable k_vt = {method_call, nullptr, nullptr, {}};

static gboolean on_new_p2p(GDBusServer*, GDBusConnection* conn, gpointer) {
	GError* err = nullptr;
	GDBusNodeInfo* info = g_dbus_node_info_new_for_xml(k_listener_xml, &err);
	if (!info) {
		std::fprintf(stderr, "qemu-egl: xml: %s\n", err ? err->message : "");
		return TRUE;
	}
	g_dbus_connection_register_object(conn, "/org/qemu/Display1/Listener",
									  info->interfaces[0], &k_vt, nullptr, nullptr, &err);
	if (err)
		std::fprintf(stderr, "qemu-egl: register object: %s\n", err->message);
	g_dbus_node_info_unref(info);
	return TRUE;
}

bool qemu_egl_connect(const char* dbus_addr) {
	GError* err = nullptr;
	GDBusConnection* bus = nullptr;
	if (dbus_addr && dbus_addr[0]) {
		bus = g_dbus_connection_new_for_address_sync(
			dbus_addr, G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, nullptr, &err);
	} else {
		bus = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
	}
	if (!bus) {
		std::fprintf(stderr, "qemu-egl: session/dbus connect: %s\n", err ? err->message : "?");
		return false;
	}

	gchar* qemu_name = g_strdup("org.qemu");
	GVariant* names = g_dbus_connection_call_sync(
		bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames",
		nullptr, G_VARIANT_TYPE("(as)"), G_DBUS_CALL_FLAGS_NONE, 3000, nullptr, nullptr);
	if (names) {
		GVariantIter* it = nullptr;
		const gchar* n = nullptr;
		g_variant_get(names, "(as)", &it);
		while (g_variant_iter_loop(it, "s", &n)) {
			if (n && std::strncmp(n, "org.qemu", 8) == 0) {
				g_free(qemu_name);
				qemu_name = g_strdup(n);
				break;
			}
		}
		g_variant_iter_free(it);
		g_variant_unref(names);
	}

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sv) != 0) {
		std::fprintf(stderr, "qemu-egl: socketpair failed\n");
		return false;
	}

	/* Hand QEMU the peer fd first, then connect to it as a D-Bus CLIENT.
	 * QEMU serves the reverse connection (it answers our AUTH with OK). */
	/* --axiss: Console_0 carries the serial text console; the graphical
	 * guest scanout (virtio-gpu) is Console_1. Try the graphical console
	 * first: QEMU only completes the reverse p2p handshake on the console
	 * that actually receives scanouts, and blocks the caller forever
	 * otherwise. */
	const char* consoles[] = {"/org/qemu/Display1/Console_1", "/org/qemu/Display1/Console_0",
							  "/org/qemu/Display1/Console"};
	GUnixFDList* fds = g_unix_fd_list_new_from_array(&sv[1], 1);
	GVariant* ret = nullptr;
	const char* reg_console = nullptr;
	for (const char* path : consoles) {
		err = nullptr;
		ret = g_dbus_connection_call_with_unix_fd_list_sync(
			bus, qemu_name, path, "org.qemu.Display1.Console", "RegisterListener",
			g_variant_new("(h)", 0), nullptr, G_DBUS_CALL_FLAGS_NONE, 8000, fds, nullptr, nullptr,
			&err);
		if (ret) {
			std::fprintf(stderr, "qemu-egl: RegisterListener ok on %s (%s)\n", path, qemu_name);
			reg_console = path;
			break;
		}
	}
	if (!ret) {
		std::fprintf(stderr,
					 "qemu-egl: RegisterListener failed: %s\n"
					 "          Is QEMU running with -display dbus,gl=on on the session bus?\n",
					 err ? err->message : "?");
		g_object_unref(fds);
		g_free(qemu_name);
		close(sv[0]);
		close(sv[1]);
		return false;
	}
	g_variant_unref(ret);
	g_object_unref(fds);

	GSocket* gs = g_socket_new_from_fd(sv[0], &err);
	if (!gs) {
		std::fprintf(stderr, "qemu-egl: g_socket_new_from_fd: %s\n", err ? err->message : "?");
		g_free(qemu_name);
		return false;
	}
	GSocketConnection* sc = G_SOCKET_CONNECTION(g_socket_connection_factory_create_connection(gs));
	gchar* guid = g_dbus_generate_guid();
	err = nullptr;
	/* QEMU is the SERVER on the reverse connection: it answers our client
	 * greeting with OK (verified against qemu 11.1.1). Connecting as
	 * SERVER here makes new_sync block forever waiting for QEMU to speak
	 * first, which it never does. Client connections take guid=NULL. */
	GDBusConnection* p2p = g_dbus_connection_new_sync(
		G_IO_STREAM(sc), nullptr, G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr,
		nullptr, &err);
	g_free(guid);
	if (!p2p) {
		std::fprintf(stderr, "qemu-egl: p2p conn: %s\n", err ? err->message : "?");
		g_free(qemu_name);
		return false;
	}
	on_new_p2p(nullptr, p2p, nullptr);
	g_dbus_connection_start_message_processing(p2p);
	/* Do not eglInitialize here: GBM/EGL can block, and OpenXR never starts.
	 * EGL is created lazily on ScanoutDMABUF. */
	{
		std::lock_guard<std::mutex> lock(g_mu);
		g_p2p = p2p;
		if (reg_console)
			g_console_path = reg_console;
		g_sbus = bus;
		g_object_ref(g_sbus);
		/* Copy before g_free below (use-after-free reads garbage). */
		g_qemu_name = qemu_name ? qemu_name : "";
	}
	g_free(qemu_name);
	return true;
}

/* Fire-and-forget input injection on the registered console's Mouse
 * interface (absolute pointer, same path VNC uses). Async: never blocks
 * the frame loop. */
static void mouse_call(const char* method, GVariant* params) {
	GDBusConnection* conn = nullptr;
	std::string name, path;
	{
		std::lock_guard<std::mutex> lock(g_mu);
		conn = g_sbus;
		name = g_qemu_name;
		path = g_console_path;
		if (conn)
			g_object_ref(conn);
	}
	if (!conn || name.empty() || path.empty()) {
		if (params)
			g_variant_unref(params);
		return;
	}
	g_dbus_connection_call(conn, name.c_str(), path.c_str(), "org.qemu.Display1.Mouse", method,
						   params, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr,
						   nullptr);
	g_object_unref(conn);
}

void qemu_mouse_abs(uint32_t x, uint32_t y) {
	std::lock_guard<std::mutex> lock(g_mu_abs);
	if (x == g_last_mx && y == g_last_my)
		return;
	/* Throttle: the guest/QEMU display path glitches under sustained
	 * high-rate pointer floods (blue-masked frames). ~15Hz is plenty for
	 * a motion-integrated cursor; buttons are unaffected. */
	static uint64_t last_ns = 0;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	uint64_t now = (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
	if (now - last_ns < 66000000ull)
		return;
	last_ns = now;
	g_last_mx = x;
	g_last_my = y;
	mouse_call("SetAbsPosition", g_variant_new("(uu)", x, y));
}

void qemu_mouse_button(uint32_t button, bool down) {
	mouse_call(down ? "Press" : "Release", g_variant_new("(u)", button));
}

bool qemu_egl_poll(uint32_t** rgba, int* w, int* h) {
	while (g_main_context_pending(nullptr))
		g_main_context_iteration(nullptr, FALSE);
	std::lock_guard<std::mutex> lock(g_mu);
	if (!g_have || g_rgba.empty())
		return false;
	*w = g_w;
	*h = g_h;
	*rgba = (uint32_t*)g_rgba.data();
	return true;
}
