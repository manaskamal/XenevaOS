#!/bin/bash
cd "$(dirname "$0")"
REPO_ROOT="$(pwd)"

source ./lib/enviorment_variables.sh
source ./lib/functions.sh
source ./lib/dist_determine.sh


prevent_privelage_run
set -e

case $1 in
    ""|-h|--help|help)show_help;exit;;
    -d|--deps|deps)

        # call print_os function to see if the os is supported by the script
        for function in ${print_os_group_id_functions[@]}; do
            $function
        done
        pause
        # initialize sudo session
        sudo_session      
        trap sudo_stop EXIT INT TERM
        source ./lib/distro/${OS_GROUP_ID}.sh
    ;;
    --llvm|--gcc|llvm|gcc)
        printf "${STY_YELLOW}[$0]: This subcommand is deprecated. build_and_run_qemu.sh is now the\n"
        printf "single build+image+QEMU script and builds the bootloader/kernel itself.\n"
        printf "Handing off to ./build_and_run_qemu.sh ...${STY_RST}\n"
        # manager.sh used to always rebuild userspace apps unconditionally
        # (via lib/llvm.sh), and its "build" sub-arg meant "also (re)build
        # initrd2.img" — the opposite of no-arg. translating both to the new
        # script's flags, which default to rebuilding the initrd every run --axiss
        NEW_ARGS=(--force-user-apps)
        case $1 in
            --llvm|llvm) NEW_ARGS+=(--llvm) ;;
            --gcc|gcc) NEW_ARGS+=(--gcc) ;;
        esac
        if [[ ! $2 =~ ^(--build|build)$ ]]; then
            NEW_ARGS+=(--force-legacy-build)
        fi
        exec ./build_and_run_qemu.sh "${NEW_ARGS[@]}"
    ;;
    *)printf "${STY_RED}Unknown subcommand \"$1\".${STY_RST}\n";show_help;exit 1;;
esac