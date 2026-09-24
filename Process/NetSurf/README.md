# NetSurf for XenevaOS (bootstrap + CSS)

Staging browser frontend with a real CSS engine (NetSurf's libcss),
not upstream NetSurf core yet (no DOM/layout/JS/TLS).

## What it is

`netsurf.exe`: Chitralekha window, URL bar + Go button, HTTP/1.0 fetch over
XEClib sockets (same path as `Process/http/curl.exe`), readability-mode
rendering with computed styles. Build with `make llvm` (needs
`Ports/CssLibs/libcssport.a` built first); the binary drops into
`Resources/resources/netsurf.exe` for initrd packing.

Keys: type/Backspace/Enter in the URL bar, Left/Right/Home/End move the
caret (block caret, drawn by the URL bar's custom paint handler), Up/Down
PgUp/PgDn scroll the page (line-windowed slice of the fetched text).

Styling (`css.c`/`css.h` select client over `Ports/CssLibs`): every element
open selects tag/class/id/descendant/inheritance rules from a UA sheet plus
page `<style>` blocks plus inline `style=""`. Applied today: `color`,
`font-size` (normal/large buckets), vertical `margin` spacing. UA defaults
give white body text, sized headings, blue links. Still text-only layout:
no CSS box model, images, JS, or HTTPS.

Like `doom.exe`, the built binary is a local artifact (`*.exe` is
git-ignored). `Scripts/Linux/build_and_run_qemu.sh` packs it by default and
`--no-netsurf` excludes it, mirroring `--no-doom`.

## What it is not (yet)

- No CSS, no layout engine, no JS (`libcss`/`libdom`/`libparserutils` unported).
- No HTTPS: Xeneva has no TLS stack. `https://` URLs get an explicit message.
  The mbedTLS port lands before encrypted fetch.
- 16 KiB page cap, text only.

## Upgrade path

1. Port mbedTLS against XEClib TCP sockets, teach the fetcher `https://`.
2. Shim musl syscalls so upstream NetSurf support libs compile.
3. Port `libparserutils`/`libwapcaplet`/`libdom`/`libcss`, swap the text
   renderer for a real layout box tree on the Chitralekha canvas.
