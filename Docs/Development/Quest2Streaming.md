# Quest 2 streaming (Linux host)

XenevaOS stays in QEMU. The Quest 2 is a display. OpenXR runs **on the host** via [WiVRn](https://wivrn.github.io/).

```
DeodhaiXR (guest scanout)
  → xeneva-xr-view (host OpenXR quad)
  → WiVRn / Monado
  → Quest 2 client
```

DeodhaiXR **does** emit OpenXR (`xrWaitFrame` / swapchains / `xrEndFrame`) when built with `OPENXR=1`. The QEMU in-process runtime (`xr_qemu.cpp`) is a stand-in for a vendor runtime on embedded hardware. It SBS-blits both eyes into the virtio-gpu scanout; WiVRn still runs on the **host**.

```bash
Scripts/Linux/build_and_run_qemu.sh --llvm --xr-demo
# headless QEMU (-display dbus + VNC + monitor sockets), starts the HMD
# viewer, then prints the viewer / client commands.
export XR_RUNTIME_JSON=/usr/share/openxr/1/openxr_wivrn.json
Tools/xeneva-xr-view/xeneva-xr-view --egl   # HMD, true framebuffer, ocular L|R
Tools/xeneva-xr-client/xeneva-xr-client     # interactive monitor (keys, info)
```

Pick the bootloader resolution yourself in `gvncviewer localhost:0`.
GOP entries (640x480–1024x768) come from firmware; `*` entries
(1280x720–1920x1080) are manual desktop overrides the GPU driver applies
independently of GOP. The viewer resizes its swapchain to whatever the
guest emits. Note: 1080p costs ~50ms/frame of guest CPU compose time
(vs ~8ms at 640x480), and dbus ships ~8MB per full frame, damage-driven.

QEMU window should show **side-by-side** eyes. `OPENXR=0` (default) keeps the single 2D desktop (viewer: add `--mono`).

## Install

PC: `flatpak install flathub io.github.wivrn.wivrn`
Headset: WiVRn from the Meta Store (Quest 2). Versions must match.

## Run

1. WiVRn dashboard + Quest client, wait until connected.
2. `Scripts/Linux/build_and_run_qemu.sh --llvm --xr-demo`: builds, boots the
   guest headless, and starts the HMD viewer. Pick the bootloader resolution
   yourself in `gvncviewer localhost:0`.
   (A child process cannot export into your shell, so it also writes
   `/tmp/xeneva-xr-demo.env` — `source` it for manual viewer runs.)
3. Interact with the OS while streaming: `gvncviewer localhost:0` (VNC
   keyboard/mouse into the guest; add `-z 50` if the 1080p window is too
   big to reach the menu button), `telnet 127.0.0.1 4444` (real QEMU
   monitor), or `Tools/xeneva-xr-client/xeneva-xr-client` (scriptable REPL:
   `.boot`, `.key ret`, `info status`, ...).
4. Showpiece mouse: `xeneva-xr-view --egl --hands` drives the guest cursor
   from right-hand tracking (aim position + pinch), `--controllers` does
   the same from the right controller (aim + trigger, wins while valid);
   combine both flags. Injected over dbus like VNC input. Needs hand
   tracking enabled in WiVRn and on the Quest for `--hands`.

Details: `Tools/xeneva-xr-view/README.md`, `Tools/xeneva-xr-client/README.md`.

## Steal frames (EGL / DMA-BUF, not VNC)

```bash
Scripts/Linux/build_and_run_qemu.sh --llvm --egl-headless
Tools/xeneva-xr-view/xeneva-xr-view --desktop --egl
```

QEMU exports `org.qemu.Display1` on `$XDG_RUNTIME_DIR/xeneva-qemu-display`. The viewer registers a Listener and prefers `ScanoutDMABUF` (EGL import + FBO read). If virtio-gpu 2D only sends pixman `Scanout`, that blob is used instead. Do not VNC-recapture.
