#!/bin/bash
# Scripts/Linux/demo_microkernel.sh -- recordable microkernel demo.
#
# Boots Xeneva in a QEMU window and shows, in this terminal, the complete serial
# console (all boot output, nothing filtered) with the interesting lines in
# color: a user-space network driver (netsrv) crashing and being restarted by
# svcmgr while the system keeps running. Put the two windows side by side and
# record. The raw serial is also saved to $DEMO_LOG
# (default /tmp/xeneva-demo-serial.log).
#
# Usage:
#   Scripts/Linux/demo_microkernel.sh              driver crashes once, svcmgr restarts it
#   Scripts/Linux/demo_microkernel.sh --fallback   driver crash-loops, in-kernel driver takes over
#
# The crash is a scripted test hook (Process/svcmgr/main.cpp), so every take
# looks the same. The build is incremental; --bleed cannot be combined with
# --skip-build, so each take reruns the normal build step. Needs a display
# session (QEMU gtk window).

set -u
DEMO_LOG=${DEMO_LOG:-/tmp/xeneva-demo-serial.log}
cd "$(dirname "$0")/../.." || exit 1

MODE=netrestart
case "${1:-}" in
	"") ;;
	--fallback) MODE=netfallback ;;
	*) echo "usage: $0 [--fallback]" >&2; exit 2 ;;
esac

RED=$'\e[1;31m'; YEL=$'\e[1;33m'; GRN=$'\e[1;32m'; MAG=$'\e[1;35m'
DIM=$'\e[2;32m'; CYN=$'\e[1;36m'; RST=$'\e[0m'

echo "${CYN}=== Xeneva microkernel demo ===${RST}"
echo "The network driver runs as a normal user-space process (netsrv)."
echo "Watch it crash. The OS stays up, svcmgr restarts it, pings keep flowing."
[ "$MODE" = netfallback ] && echo "(fallback mode: it keeps crashing, so the in-kernel driver takes over)"
echo "Full serial log is saved to ${DEMO_LOG}"
echo "Building (build output hidden), then the full serial console follows..."
echo

export NETSRV=1 SVCTEST="$MODE"
booted=0
# 1. tee keeps the raw serial for later
# 2. sed strips terminal control codes: the UEFI firmware sends clear-screen
#    sequences that would wipe this terminal in the middle of a recording
# 3. the loop shows every serial line, coloring the ones that tell the story;
#    -u / read -r keep it line-buffered so lines appear live on camera
Scripts/Linux/build_and_run_qemu.sh --llvm --bleed 2>&1 | tee "$DEMO_LOG" \
		| sed -u -e 's/\x1b\[[0-9;?=]*[A-Za-z]//g' -e 's/\x1b[()][A-Z0-9]//g' | while IFS= read -r line; do
	line=${line//$'\r'/}
	# everything before the firmware banner is build output
	if [ "$booted" -eq 0 ]; then
		case "$line" in
			"UEFI firmware"*) booted=1; SECONDS=0 ;;
			*) continue ;;
		esac
	fi
	case "$line" in
		*">>>"*"is DOWN"*)       c=$RED ;;
		*">>>"*"RESTART"*)       c=$YEL ;;
		*">>>"*"is BACK"*)       c=$GRN ;;
		*">>>"*"FALLING BACK"*)  c=$MAG ;;
		*">>>"*"FAILED"*)        c=$RED ;;
		*"[svcmgr]"*)            c=$YEL ;;
		*"[netsrv]"*)            c=$CYN ;;
		*"[nettest]"*"reply"*)   c=$DIM ;;
		*"[nettest]"*"timeout"*|*"[nettest]"*"busy"*) c=$RED ;;
		*) c= ;;
	esac
	if [ -n "$c" ]; then
		# $SECONDS is bash's own timer, zeroed at the firmware banner
		printf '%s[%3ds] %s%s\n' "$c" "$SECONDS" "$line" "$RST"
	else
		printf '%s\n' "$line"
	fi
done
