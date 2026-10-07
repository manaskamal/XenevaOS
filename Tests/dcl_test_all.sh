#!/usr/bin/env bash
# Tests/dcl_test_all.sh -- the gate for the DCL tty/serial port.
#
# "Green here" is what a finished stage means: every stage ends by running
# this. Four parts, in order, each reporting `ok` or stopping with a reason:
#
#   1. tree counts  -- a file appearing or disappearing should be a decision
#                      with a number attached, not something noticed three
#                      stages later
#   2. build        -- full `make llvm`, grepped for `error:`. Worth being
#                      explicit: `make llvm | tail` reports *tail's* exit
#                      status, so pipefail alone would pass a failed build
#   3. boot         -- a complete QEMU run. Exit 124 (timeout) is the
#                      expected status; the verdict is the log
#   4. assertions   -- on that log
#
# Usage:
#   ./Tests/dcl_test_all.sh              full run
#   CLEAN=1 ./Tests/dcl_test_all.sh      `make clean` first -- required after any edit
#                                        under BaseHdr/, because KernelAA64/Makefile has
#                                        no header dependency tracking and stale objects
#                                        will silently keep old header contents
#
set -uo pipefail

cd "$(dirname "$0")/.." || exit 1

# ─── expected tree shape ────────────────────────────────────────────────────
# Bump deliberately, in the same change that alters the count.
EXP_LINUX_H=109    # BaseHdr/linux/*.h
EXP_ASM_H=4        # BaseHdr/asm/*.h
EXP_DCL_C=14       # DCL/*.c   (dcl_va_trampoline.s adds the 15th object)
EXP_DCL_O=15       # KernelAA64/obj/DCL/*.o
EXP_VEND_O=8       # KernelAA64/obj/Vendored/**/*.o  (stage 3's tty/serial)
EXP_ALL_O=141      # everything under KernelAA64/obj

# ─── expected boot-log results ──────────────────────────────────────────────
EXP_MEM=9
EXP_PRIM=10
EXP_TTYBUF=25
EXP_SERIAL=21
EXP_DCLTEST=27

LOG=${LOG:-/tmp/xeneva/test_all.log}
mkdir -p "$(dirname "$LOG")"

fails=0
ok()  { printf '  %-42s ok\n' "$1"; }
bad() { printf '  %-42s FAILED (%s)\n' "$1" "${2:-}"; fails=$((fails + 1)); }

check_count() {  # label actual expected
	if [ "$2" -eq "$3" ]; then ok "$1 = $2"; else bad "$1" "got $2, want $3"; fi
}

# ─── 1. tree counts ─────────────────────────────────────────────────────────
echo "tree counts"
check_count "BaseHdr/linux headers" "$(ls BaseHdr/linux/*.h | wc -l)"            "$EXP_LINUX_H"
check_count "BaseHdr/asm headers"   "$(ls BaseHdr/asm/*.h | wc -l)"              "$EXP_ASM_H"
check_count "DCL sources"           "$(ls DCL/*.c | wc -l)"                      "$EXP_DCL_C"
check_count "DCL objects"           "$(ls KernelAA64/obj/DCL/*.o 2>/dev/null | wc -l)" "$EXP_DCL_O"
check_count "Vendored objects"      "$(find KernelAA64/obj/Vendored -name '*.o' 2>/dev/null | wc -l)" "$EXP_VEND_O"
check_count "total objects"         "$(find KernelAA64/obj -name '*.o' | wc -l)" "$EXP_ALL_O"

# A vendored source must still be what MANIFEST.md says it is. This is check 1
# from that file, inlined so a run of this gate cannot pass with a silently
# edited vendored file. The manifest row carries two md5s (upstream, then
# local) so `tail -1` takes the local one; the row is picked out by its tag
# cell rather than by the path, since the path also appears in a heading below.
if [ -f Vendored/MANIFEST.md ]; then
	_manifest=$(grep -F '`v7.2`' Vendored/MANIFEST.md | head -1 |
		grep -oE '[0-9a-f]{32}' | tail -1)
	_actual=$(md5sum Vendored/drivers/char/hw_random/virtio-rng.c | cut -d' ' -f1)
	if [ -n "$_manifest" ] && [ "$_manifest" = "$_actual" ]; then
		ok "vendored virtio-rng.c matches MANIFEST.md"
	else
		bad "vendored virtio-rng.c" "manifest ${_manifest:-<none>}, file $_actual"
	fi
fi

[ "$fails" -eq 0 ] || { echo "count assertions failed; not booting"; exit 1; }

# ─── 2 + 3. build and boot ──────────────────────────────────────────────────
echo
echo "build + boot (log: $LOG)"
if [ -n "${CLEAN:-}" ]; then
	echo "  CLEAN=1 -> make clean"
	make -C KernelAA64 clean >/dev/null 2>&1
fi

# stdin from /dev/null rather than inherited. `-serial stdio` makes the
# guest's console this process's stdin, and a TTY there gives a guest that
# prints nothing for the whole timeout: QEMU exits 124 as expected and the
# log ends at the banner, so every assertion below fails on a boot that
# never happened. Verified -- the same run green on a non-TTY stdin, 17
# FAILED on a pty.
QEMU_TIMEOUT="${QEMU_TIMEOUT:-120}" ./Scripts/Linux/build_and_run_qemu.sh \
	--llvm --headless --no-bt --no-audio >"$LOG" 2>&1 </dev/null
rc=$?
case $rc in
	124) ok "QEMU ran to timeout (124, expected)" ;;
	0)   ok "QEMU exited 0" ;;
	*)   bad "QEMU exit status" "got $rc, want 124 or 0" ;;
esac

# `make | tail` masks failures -- check the log for the compiler's own verdict.
if grep -q 'error:' "$LOG"; then
	bad "build has no error:" "$(grep -m1 'error:' "$LOG" | cut -c1-100)"
else
	ok "build has no error:"
fi

# ─── 4. assertions ──────────────────────────────────────────────────────────
echo
echo "boot log"

# A guest that printed nothing is a different failure from a guest whose
# assertions failed, and the 17 `missing:` lines below would bury that
# distinction. Guest lines only exist past QEMU's banner; grepping the whole
# log would let a quoted UARTDebugOut inside a build warning count as boot
# output, so split at the banner first.
if ! awk '/Image ready! Booting QEMU/{s=1} s && /\[aurora\]|\[dcl\]|\[modtest\]|\[pmm\]/{f=1} END{exit f?0:1}' "$LOG"; then
	bad "guest produced output" "no boot lines after the QEMU banner"
	echo
	echo "test_all: $fails FAILED"
	exit 1
fi

expect() {   # label pattern
	if grep -qF -- "$2" "$LOG"; then ok "$1"; else bad "$1" "missing: $2"; fi
}

expect "chr_dev_init"                       "chr_dev_init ok (mem devices registered)"
expect "mem devices"                        "mem devices $EXP_MEM ok, 0 failed"
expect "stage 1 primitives"                 "dcl primitives: $EXP_PRIM ok, 0 failed"
expect "stage 2 tty_buffer"                 "dcl tty_buffer: $EXP_TTYBUF ok, 0 failed"
expect "stage 3 serial core"                "dcl serial: $EXP_SERIAL ok, 0 failed"
expect "userspace dcltest"                  "[dcltest] SUMMARY $EXP_DCLTEST ok, 0 failed"

# The vendored driver, compiled from source into the image.
expect "vendored driver registered"         "[dcl]: virtio_rng driver registered"
expect "vendored driver is not a .ko"       "[modtest]: virtio_rng built-in"
expect "probe ran"                          "[dcl-virtio]: probe done"
expect "/dev/hwrng registered"              "[dcl]: hwrng_register (virtio_rng.0)"
expect "/dev/hwrng round-trip"              "[dcl]: /dev/hwrng round-trip OK (samples differ)"

# The ELF loader is a separate guarantee from "the driver links" -- it still
# loads and runs test_module.o through DCL/module_loader.c.
expect "ELF loader still exercised"         "[modtest] init returned  0"

# Nothing anywhere may be failing.
if grep -qE '[1-9][0-9]* failed|FAILED' "$LOG"; then
	bad "no failures anywhere" "$(grep -m1 -E '[1-9][0-9]* failed|FAILED' "$LOG" | cut -c1-100)"
else
	ok "no failures anywhere"
fi
if grep -qiE '^[a-z].*\b(panic|unhandled)\b' "$LOG"; then
	bad "no panic/unhandled" "$(grep -m1 -iE '^[a-z].*\b(panic|unhandled)\b' "$LOG" | cut -c1-100)"
else
	ok "no panic/unhandled"
fi

# ─── digest ─────────────────────────────────────────────────────────────────
echo
echo "digest"
python3 ./ts_filter.py "$LOG" 2>/dev/null || echo "  (ts_filter.py unavailable)"

echo
if [ "$fails" -eq 0 ]; then
	echo "test_all: ALL GREEN"
	exit 0
fi
echo "test_all: $fails FAILED"
exit 1
