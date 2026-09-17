#!/bin/bash
set -e

# XR demo policy/UI module. It only selects components and execs the regular
# build/QEMU runner; the mechanisms remain independently usable. --axiss

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUNNER="$SCRIPT_DIR/build_and_run_qemu.sh"

reset_defaults() {
    build_preset="llvm"
    hands=1
    controllers=1
    hand_mesh=1
    resolution="1024x768"
    gain="20000"
    scaling="sharp"
    vnc=1
    telnet_monitor=0
    unikernel=0
    direct_scanout=0
    soak=0
}

reset_defaults

# The main runner exports parsed command-line choices before delegating here,
# so `--xr-demo --gcc --xr-no-hands` opens with those choices selected.
build_preset="${XENEVA_XR_TUI_BUILD:-$build_preset}"
hands="${XENEVA_XR_TUI_HANDS:-$hands}"
controllers="${XENEVA_XR_TUI_CONTROLLERS:-$controllers}"
hand_mesh="${XENEVA_XR_TUI_HAND_MESH:-$hand_mesh}"
resolution="${XENEVA_XR_TUI_RESOLUTION:-$resolution}"
gain="${XENEVA_XR_TUI_GAIN:-$gain}"
scaling="${XENEVA_XR_TUI_SCALE:-$scaling}"
vnc="${XENEVA_XR_TUI_VNC:-$vnc}"
telnet_monitor="${XENEVA_XR_TUI_TELNET:-$telnet_monitor}"
unikernel="${XENEVA_XR_TUI_UNIKERNEL:-$unikernel}"
direct_scanout="${XENEVA_XR_TUI_DIRECT_SCANOUT:-$direct_scanout}"
soak="${XENEVA_XR_TUI_SOAK:-$soak}"

build_command() {
    XR_COMMAND=("$RUNNER" --xr-demo-defaults)
    case "$build_preset" in
        llvm) XR_COMMAND+=(--llvm) ;;
        gcc) XR_COMMAND+=(--gcc) ;;
        bleed) XR_COMMAND+=(--llvm --bleed) ;;
    esac
    [ "$hands" -eq 1 ] && XR_COMMAND+=(--xr-hands) || XR_COMMAND+=(--xr-no-hands)
    [ "$controllers" -eq 1 ] && XR_COMMAND+=(--xr-controllers) || XR_COMMAND+=(--xr-no-controllers)
    [ "$hand_mesh" -eq 1 ] && XR_COMMAND+=(--xr-hand-mesh) || XR_COMMAND+=(--xr-no-hand-mesh)
    XR_COMMAND+=("--xr-resolution=$resolution" "--xr-gain=$gain" "--xr-scale=$scaling")
    [ "$vnc" -eq 1 ] && XR_COMMAND+=(--xr-vnc) || XR_COMMAND+=(--xr-no-vnc)
    [ "$telnet_monitor" -eq 1 ] && XR_COMMAND+=(--xr-telnet) || XR_COMMAND+=(--xr-no-telnet)
    [ "$unikernel" -eq 1 ] && XR_COMMAND+=(--unikernel)
    [ "$direct_scanout" -eq 1 ] && XR_COMMAND+=(--direct-scanout)
    [ "$soak" -eq 1 ] && XR_COMMAND+=(--soak)
    return 0
}

print_command() {
    build_command
    printf '%q ' "${XR_COMMAND[@]}"
    printf '\n'
}

case "${1:-}" in
    --print-defaults)
        print_command
        exit 0
        ;;
    -h|--help)
        echo "usage: Scripts/Linux/xr-demo.sh [--print-defaults]"
        echo "       Scripts/Linux/build_and_run_qemu.sh --xr-demo"
        exit 0
        ;;
esac

if [ ! -t 0 ] || [ ! -t 1 ]; then
    echo "xr-demo: interactive TUI needs a terminal." >&2
    echo "Use --xr-demo-defaults for the recommended noninteractive profile." >&2
    exit 2
fi

items=(build hands controllers hand_mesh resolution gain scaling vnc telnet unikernel direct_scanout soak launch quit)
selected=0

on_off() {
    if [ "$1" -eq 1 ]; then
        printf '\033[1;32mON \033[0m'
    else
        printf '\033[2;31moff\033[0m'
    fi
}

cycle_build() {
    case "$build_preset" in
        llvm) build_preset="gcc" ;;
        gcc) build_preset="bleed" ;;
        bleed) build_preset="llvm" ;;
    esac
}

cycle_resolution() {
    case "$resolution" in
        1024x768) resolution="800x600" ;;
        800x600) resolution="640x480" ;;
        640x480) resolution="manual" ;;
        manual) resolution="1024x768" ;;
    esac
}

cycle_gain() {
    case "$gain" in
        12000) gain="20000" ;;
        20000) gain="30000" ;;
        30000) gain="12000" ;;
    esac
}

cycle_scaling() {
    [ "$scaling" = sharp ] && scaling="smooth" || scaling="sharp"
}

activate() {
    case "${items[$selected]}" in
        build) cycle_build ;;
        hands) hands=$((1 - hands)) ;;
        controllers) controllers=$((1 - controllers)) ;;
        hand_mesh) hand_mesh=$((1 - hand_mesh)) ;;
        resolution) cycle_resolution ;;
        gain) cycle_gain ;;
        scaling) cycle_scaling ;;
        vnc) vnc=$((1 - vnc)) ;;
        telnet) telnet_monitor=$((1 - telnet_monitor)) ;;
        unikernel) unikernel=$((1 - unikernel)) ;;
        direct_scanout) direct_scanout=$((1 - direct_scanout)) ;;
        soak) soak=$((1 - soak)) ;;
        launch)
            restore_terminal
            build_command
            exec "${XR_COMMAND[@]}"
            ;;
        quit)
            restore_terminal
            exit 0
            ;;
    esac
}

row() {
    local index="$1" label="$2" value="$3"
    if [ "$selected" -eq "$index" ]; then
        printf '\033[48;5;24m\033[38;5;231m│ \033[0m'
        printf '  %-24s ' "$label"
        printf '%s\n' "$value"
    else
        printf '  \033[2m│\033[0m '
        printf '  %-24s ' "$label"
        printf '%s\n' "$value"
    fi
}

render() {
	local pipeline services
	if [ "$vnc" -eq 1 ]; then
		pipeline="QEMU CPU scanout → D-Bus → OpenXR → WiVRn"
		services="D-Bus CPU scanout + VNC + Unix monitor + WiVRn"
	else
		pipeline="QEMU DMA-BUF → EGL → OpenXR → WiVRn"
		services="EGL headless + QEMU D-Bus + Unix monitor + WiVRn"
	fi

	local w=62
	printf '\033[?25l'
	printf '\033[H\033[2J'
	printf '\033[1;36m┌%s┐\033[0m\n' "$(printf '%*s' "$w" | tr ' ' '─')"
	printf '\033[1;36m│\033[0m \033[1mXENEVA XR DEMO\033[0m'
	printf '%*s\033[1;36m │\033[0m\n' "$((w - ${#pipeline} - 4))" "modular host launcher"
	printf '\033[1;36m│\033[0m'
	printf '%*s\033[1;36m │\033[0m\n' "$((w - ${#pipeline} - 4))" ""
	printf '\033[2m│\033[0m  %s' "$pipeline"
	printf '%*s\033[2m │\033[0m\n' "$((w - ${#pipeline} - 4))" ""
	printf '\033[1;36m├%s┤\033[0m\n' "$(printf '%*s' "$w" | tr ' ' '─')"
	row 0  "Build preset" "$build_preset"
	row 1  "Hand pointer / pinch" "$(on_off "$hands")"
	row 2  "VR controller / trigger" "$(on_off "$controllers")"
	row 3  "Tracked hand mesh" "$(on_off "$hand_mesh")"
	row 4  "Guest resolution" "$resolution"
	row 5  "Pointer gain" "${gain} px/m"
	row 6  "Scaling" "$scaling"
	row 7  "VNC service" "$(on_off "$vnc")"
	row 8  "Telnet monitor" "$(on_off "$telnet_monitor")"
	row 9  "Unikernel shell" "$(on_off "$unikernel")"
	row 10 "Direct scanout" "$(on_off "$direct_scanout")"
	row 11 "Scheduler soak" "$(on_off "$soak")"
	row 12 "Launch" "build + supervise"
	row 13 "Quit" ""
	printf '\033[1;36m└%s┘\033[0m\n' "$(printf '%*s' "$w" | tr ' ' '─')"
	printf '\n  \033[2mCore: %s\033[0m\n' "$services"
	printf '  \033[1;33m↑↓\033[0m select  \033[1;33m⏎\033[0m change  \033[1;33mD\033[0m defaults  \033[1;33mQ\033[0m quit\n'
}

restore_terminal() {
    printf '\033[?25h\033[0m'
}
trap restore_terminal EXIT INT TERM
printf '\033[?25l'

while true; do
    render
    IFS= read -rsn1 key
    if [ "$key" = $'\e' ]; then
        IFS= read -rsn2 -t 0.1 rest || true
        key+="$rest"
    fi
    case "$key" in
        $'\e[A'|k) selected=$(( (selected + ${#items[@]} - 1) % ${#items[@]} )) ;;
        $'\e[B'|j) selected=$(( (selected + 1) % ${#items[@]} )) ;;
        ' '|$'\n'|'') activate ;;
        d|D) reset_defaults ;;
        q|Q) restore_terminal; exit 0 ;;
    esac
done
