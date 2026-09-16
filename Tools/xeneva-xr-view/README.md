# xeneva-xr-view

Host-only OpenXR tool: shows the live QEMU desktop as a world-locked quad in a Quest 2 (or any OpenXR HMD). **Not** part of the guest image. The XR runtime stays on Linux (WiVRn).

## Stage 0 — headset to PC

```bash
flatpak install flathub io.github.wivrn.wivrn
flatpak run io.github.wivrn.wivrn
```

On the Quest 2, install **WiVRn** from the Meta Store (same version as the PC). Connect over 5 GHz Wi-Fi or USB+ADB. Confirm a stock OpenXR sample before involving Xeneva.

WiVRn sets `XR_RUNTIME_JSON` while the headset is connected.

## Laptop preview (no headset)

```bash
make -C Tools/xeneva-xr-view
# live QEMU GTK window (title matching qemu, case-insensitive):
Tools/xeneva-xr-view/xeneva-xr-view --desktop --x11
# stereo test chart only (checker + yellow disc, not a rainbow):
Tools/xeneva-xr-view/xeneva-xr-view --desktop --pattern
```

Preferred capture (no X11, no VNC): QEMU `egl-headless` + dbus DMA-BUF.

```bash
Scripts/Linux/build_and_run_qemu.sh --llvm --egl-headless
Tools/xeneva-xr-view/xeneva-xr-view --desktop --egl
```

That imports `ScanoutDMABUF` through EGL (`eglCreateImageKHR` / `GL_TEXTURE_2D`) and falls back to the CPU `Scanout` blob if QEMU does not export a dmabuf (2D virtio-gpu without virgl).

Side-by-side by default (white seam down the middle, green near-squares offset by IPD). `--anaglyph` for red/cyan. `[` `]` IPD, `s` layout, `q` quit. This is a fake two-eye shift of one 2D plane, not Quest depth.

## Stage 1 — Xeneva as a floating panel

1. Boot the guest: `Scripts/Linux/build_and_run_qemu.sh --llvm`
2. Leave the QEMU GTK window visible (title contains `QEMU`).
3. With WiVRn connected:

```bash
make -C Tools/xeneva-xr-view
Tools/xeneva-xr-view/xeneva-xr-view --x11
```

Optional:

- `--window SUBSTR`  match a different window title
- `--ppm FILE`       read a `P6` PPM (for a dump path later)
- `--pattern`        RGB test card (no QEMU)
- `--size WxH`       test-pattern size

The quad sits about 2 m in front of `LOCAL` space, both eyes, same texture. Head motion is applied by the runtime (timewarp). The guest does not need IMU yet.

With an `OPENXR=1` guest the scanout is side-by-side L|R and the viewer
shows each eye its own half (ocular, `--stereo`, the default). For a plain
2D guest pass `--mono`.

## Headless (xr-demo)

```bash
Scripts/Linux/build_and_run_qemu.sh --llvm --xr-demo
export XR_RUNTIME_JSON=/usr/share/openxr/1/openxr_wivrn.json
Tools/xeneva-xr-view/xeneva-xr-view --egl   # true guest framebuffer over dbus
Tools/xeneva-xr-client/xeneva-xr-client     # interactive monitor (keys, info)
```

No GTK window: QEMU runs `-display dbus` (+ VNC) with a monitor socket.
Pick the bootloader resolution yourself in `gvncviewer localhost:0`.
`--egl` steals the real scanout (CPU `Scanout` blobs, or DMA-BUF when EGL
import works), not an X11 screenshot.

## Stage 2 (not this tool)

Stereo IPD split is a DeodhaiXR side-by-side dump plus two projection views. Do that only after Stage 1 is readable in the headset.

## Dependencies

`libopenxr-loader`, X11, GLX (`pkg-config openxr x11 gl`).
