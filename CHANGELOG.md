# Changelog

## Unreleased

### Fixed

- **fix(BootAA64)**: `ExitBootServices` retry loop maybe livelock through all 16 retries and hang before the kernel ever ran — the `EFI_BUFFER_TOO_SMALL`
  reallocation path gave the memory-map buffer zero headroom, unlike the
  initial allocation, so the `FreePool`/`AllocatePool` churn could itself
  perturb the map and trigger the same failure again. Reproduced
  intermittently (2/5 headless boots hung); fixed by matching the initial
  allocation's headroom. (`BootAA64/xnldr.cpp`)

- **fix(Mm/tlsf)**: `tlsf_fls()` computed size-class buckets using
  `__builtin_clzl()` against a `size_t`, but the kernel targets
  `aarch64-unknown-windows` where `long` is 32-bit — every bucket index was
  wrong, so `tlsf_malloc()` could never find memory that provably existed in
  the pool. `kmalloc()` failed on the very first real allocation after boot.
  (`KernelAA64/Mm/tlsf.c`)

- **fix(Mm/tlsf)**: `tlsf_malloc()` removed the found free block from the
  free-list bucket computed from the *requested* size instead of the block's
  *actual* size. Since the search can (correctly) fall back to a larger
  bucket, this frequently removed from the wrong index — a silent no-op —
  leaving the block registered as free while already handed out, causing
  double-allocation on the next call. (`KernelAA64/Mm/tlsf.c`)

- **fix(Mm/tlsf)**: `tlsf_malloc()`/`tlsf_realloc()` treated the caller's
  requested payload size as the total block size, never adding room for the
  16-byte block header. Every allocation returned a buffer 16 bytes smaller
  than requested; allocations landing close to a block boundary silently
  overflowed into the next block's header, corrupting heap metadata (root
  cause of the intermittent post-boot crashes / "memory management" hangs).
  (`KernelAA64/Mm/tlsf.c`)

- **fix(list)**: `initialize_list()` / `list_add()` dereferenced `kmalloc()`'s result without checking for `NULL`, turning any allocation failure into an unrecoverable NULL-pointer crash instead of a graceful failure. (`KernelAA64/list.c`)

- **fix(Init)**: `SplashScreenShow()` opened `/xelogo.bmp` and went through file-stat, mmap, and BMP-header parsing without checking any intermediate result for failure. A missing/unreadable logo file crashed `init.exe` outright on boot. Now it just skips the thing if missing. (`Process/Init/splash.cpp`)

- **fix(Net/socket, Mm/mmap)**: two call sites allocated `kmalloc(strlen(x))` immediately before `strcpy(dst, x)`, one byte short of the null terminator, corrupting the next heap block's header by one byte on every call. (`KernelAA64/Net/socket.c`, `KernelAA64/Mm/mmap.c`)

### Added

- `Scripts/Linux/gen_compile_commands.sh` output wired up for clangd across
  the bootloadder, kernel, both userspace libraries, and every app under
  `Process/`.

### Known issues

- `Process/Control/ctrl.exe` crashes on a non-null fault address partway through window creation. Zero idea what's causing it, my guess is off-by-one. type 3.
- `BootAA64/xnldr.cpp`'s EBS-failure diagnostic casts a `char*` string literal straight to `wchar_t*` without conversion, producing garbled console output. Shouldn't be a prob, but worth fixing at some point in timme.
- `BootAA64/xnldr.cpp`'s `_need_fdt_hardcode` fallback only triggers on an FDT magic *mismatch*, not on the FDT being literally gone from the UEFI configuration table. Maybe put on the tracker.
- `/xelogo.bmp` referenced by `Process/Init/splash.cpp` is missing??
