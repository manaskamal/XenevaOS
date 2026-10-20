#! This script is not meant for execution, so no need for execution permission or shebang.

printf "${STY_CYAN}compiling Xeneva for AArch64 using LLVM/Clang.${STY_RST}\n"

# The EFI bootloader needs the gnu-efi sources (headers) checked out next to
# the repository root. See Docs/BuildInstructions(Linux).md.
if [ ! -d ../../gnu-efi ]; then
    printf "${STY_RED}[llvm] gnu-efi not found at repo-root/gnu-efi.${STY_RST}\n"
    printf "${STY_YELLOW}[llvm] Clone it first: git clone https://github.com/vathpela/gnu-efi.git${STY_RST}\n"
    exit 1
fi

# EFI bootloader
( cd ../../BootAA64 && make clean && make BLEED="${BLEED:-0}" llvm )

# AArch64 kernel
( cd ../../KernelAA64 && make clean && make BLEED="${BLEED:-0}" SOAK="${SOAK:-0}" DEBUG_ALLOC="${DEBUG_ALLOC:-0}" STATIC_COMPONENTS="${STATIC_COMPONENTS:-0}" llvm )

if [ "${BUILD_USER_APPS:-0}" -eq 1 ]; then
    # Userspace C++ runtime + graphics library
    ( cd ../../Libs/XEClib && make clean && make BLEED="${BLEED:-0}" llvm )
    ( cd ../../Libs/Chitralekha && make clean && make BLEED="${BLEED:-0}" UNIKERNEL="${UNIKERNEL:-0}" llvm )
    if [ "${NO_NETWORK:-0}" -eq 0 ]; then
        ( cd ../../Ports/mbedtls && make clean && make llvm )
    fi

    # All AArch64 user-space applications. Microkernel rule: --no-network
    # and --no-audio drop their whole userspace (daemons and tools) so a
    # removed component is never built, deployed, or packed. --axiss
    APPS=(
        Init DeodhaiXR Terminal Namdapha XELnch
        Calender Calculator Files Control XEShell btctl XENotes XEPaints XESheets
        Clipd dcltest
    )
    # hwtest unloads the in-kernel sound driver and claims its PCI function from
    # user space, so it is only built and packed when explicitly requested:
    #   HWTEST=1 ./Scripts/Linux/build_and_run_qemu.sh --llvm ...
    if [ "${HWTEST:-0}" -eq 1 ]; then
        APPS+=(hwtest)
    fi
    # sndsrv replaces the in-kernel sound driver with a user-space one; sndplay
    # plays a test tone through it. Both are opt-in:  SNDSRV=1 ./Scripts/...
    if [ "${SNDSRV:-0}" -eq 1 ]; then
        APPS+=(sndsrv sndplay svcmgr)
        if [ -n "${SVCTEST:-}" ]; then APPS+=(svcdummy); fi    # test-only short-lived service
    elif [ "${SNDPLAY:-0}" -eq 1 ]; then
        APPS+=(sndplay)       # tone test against the in-kernel driver
    fi
    # inputsrv replaces the kernel's virtio keyboard/tablet drivers with a user-space
    # one, supervised by svcmgr:  INPUTSRV=1 ./Scripts/Linux/build_and_run_qemu.sh ...
    if [ "${INPUTSRV:-0}" -eq 1 ]; then
        APPS+=(inputsrv)
        if [ "${SNDSRV:-0}" -ne 1 ]; then APPS+=(svcmgr); fi
    fi
    # generic IPC test: a daemons-group echo service and an ordinary-app client
    if [ "${IPCTEST:-0}" -eq 1 ]; then APPS+=(ipcsrv ipctest); fi
    # user-space file system on /srv plus its two-user test:  SRVTEST=1
    if [ "${SRVTEST:-0}" -eq 1 ]; then
        APPS+=(fssrv srvtest)
        if [ "${SNDSRV:-0}" -ne 1 ] && [ "${INPUTSRV:-0}" -ne 1 ] && [ "${NETSRV:-0}" -ne 1 ]; then APPS+=(svcmgr); fi   # fssrv runs under the supervisor
    fi
    # netsrv replaces the kernel's virtio-net driver with a user-space one, supervised
    # by svcmgr; nettest pings the gateway every 2 s:  NETSRV=1 ./Scripts/...
    if [ "${NETTEST:-0}" -eq 1 ] && [ "${NETSRV:-0}" -ne 1 ]; then APPS+=(nettest); fi   # probe alone
    if [ "${NETSRV:-0}" -eq 1 ]; then
        APPS+=(netsrv nettest)
        if [ "${SNDSRV:-0}" -ne 1 ] && [ "${INPUTSRV:-0}" -ne 1 ]; then APPS+=(svcmgr); fi
    fi
    if [ "${NO_AUDIO:-0}" -eq 0 ]; then
        APPS+=(DeodhaiAudio AudioPlayer)
    else
        echo "[llvm] Audio userspace excluded (--no-audio)."
        rm -f ../../Resources/resources/deoaud.exe ../../Resources/resources/audplr.exe
    fi
    if [ "${NO_NETWORK:-0}" -eq 0 ]; then
        APPS+=(ping udpecho route iptables NETMngr dig nslook Telnet Finger Gopher Ntpd wifictl)
    else
        echo "[llvm] Network userspace excluded (--no-network)."
        rm -f ../../Resources/resources/ping.exe ../../Resources/resources/udpecho.exe \
            ../../Resources/resources/route.exe ../../Resources/resources/iptable.exe \
            ../../Resources/resources/netmngr.exe ../../Resources/resources/dig.exe \
            ../../Resources/resources/nslook.exe ../../Resources/resources/telnet.exe \
            ../../Resources/resources/finger.exe ../../Resources/resources/gopher.exe \
            ../../Resources/resources/ntpd.exe ../../Resources/resources/curl.exe \
            ../../Resources/resources/wifictl.exe \
            ../../Resources/resources/netsurf.exe
    fi
    for app in "${APPS[@]}"; do
        ( cd "../../Process/$app" && make clean && make BLEED="${BLEED:-0}" UNIKERNEL="${UNIKERNEL:-0}" DIRECT_SCANOUT="${DIRECT_SCANOUT:-0}" OPENXR="${OPENXR:-0}" llvm )
    done

    # Deploy the freshly built application binaries into the resources tree so
    # they get packed into initrd2.img by the caller's resource-copy step.
    cp -f ../../Process/Init/init.exe            ../../Resources/resources/
    cp -f ../../Process/DeodhaiXR/deodxr.exe     ../../Resources/resources/
    cp -f ../../Process/Terminal/term.exe         ../../Resources/resources/
    cp -f ../../Process/Namdapha/nmdapha.exe      ../../Resources/resources/
    cp -f ../../Process/XELnch/xelnch.exe         ../../Resources/resources/
    cp -f ../../Process/Calender/calendr.exe      ../../Resources/resources/
    cp -f ../../Process/Calculator/calc.exe       ../../Resources/resources/
    cp -f ../../Process/Files/file.exe            ../../Resources/resources/
    cp -f ../../Process/Control/ctrl.exe          ../../Resources/resources/
    cp -f ../../Process/XEShell/xesh.exe          ../../Resources/resources/
    cp -f ../../Process/btctl/btctl.exe           ../../Resources/resources/
    cp -f ../../Process/XENotes/xenotes.exe       ../../Resources/resources/
    cp -f ../../Process/XEPaints/xepaints.exe     ../../Resources/resources/
    cp -f ../../Process/XESheets/xesheets.exe     ../../Resources/resources/
    cp -f ../../Process/Clipd/clipd.exe           ../../Resources/resources/
    cp -f ../../Process/dcltest/dcltest.exe       ../../Resources/resources/
    if [ "${HWTEST:-0}" -eq 1 ]; then
        cp -f ../../Process/hwtest/hwtest.exe     ../../Resources/resources/
    else
        rm -f ../../Resources/resources/hwtest.exe
    fi
    if [ "${SNDSRV:-0}" -eq 1 ]; then
        cp -f ../../Process/sndsrv/sndsrv.exe     ../../Resources/resources/
        cp -f ../../Process/sndplay/sndplay.exe   ../../Resources/resources/
        cp -f ../../Process/svcmgr/svcmgr.exe     ../../Resources/resources/
    elif [ "${SNDPLAY:-0}" -eq 1 ]; then
        rm -f ../../Resources/resources/sndsrv.exe
        cp -f ../../Process/sndplay/sndplay.exe   ../../Resources/resources/
    else
        rm -f ../../Resources/resources/sndsrv.exe ../../Resources/resources/sndplay.exe
    fi
    # Service table for svcmgr (one service per line, see Process/svcmgr/main.cpp).
    # SVCTEST=restart|fallback|recover|selfcrash additionally builds the test hooks
    # into sndsrv/svcmgr, packs a short-lived dummy service and writes /svctest.cnf.
    RES=../../Resources/resources
    if [ "${SNDSRV:-0}" -eq 1 ] || [ "${INPUTSRV:-0}" -eq 1 ] || [ "${NETSRV:-0}" -eq 1 ] || [ "${SRVTEST:-0}" -eq 1 ]; then
        cp -f ../../Process/svcmgr/svcmgr.exe "$RES/"
        {
            echo "# name path handback max_restarts window_s cooldown_s stable_s groups"
            if [ "${SNDSRV:-0}" -eq 1 ]; then echo "sndsrv /sndsrv.exe load_virtsnd 3 60 30 20 daemons"; fi
            if [ "${INPUTSRV:-0}" -eq 1 ]; then echo "inputsrv /inputsrv.exe load_virtio-kbd;load_virtio-tablet 3 60 30 20 daemons"; fi
            if [ "${SRVTEST:-0}" -eq 1 ]; then echo "fssrv /fssrv.exe - 3 60 2 4 daemons,world"; fi
            if [ "${NETSRV:-0}" -eq 1 ]; then echo "netsrv /netsrv.exe load_virtnet 3 60 30 20 daemons"; fi
            if [ -n "${SVCTEST:-}" ] && [ "${SNDSRV:-0}" -eq 1 ]; then echo "svcdummy /svcdummy.exe - 2 60 30 20 -"; fi
        } > "$RES/services.cnf"
        if [ -n "${SVCTEST:-}" ]; then
            printf '%s' "${SVCTEST}" > "$RES/svctest.cnf"
            if [ "${SNDSRV:-0}" -eq 1 ]; then cp -f ../../Process/svcdummy/svcdummy.exe "$RES/"; else rm -f "$RES/svcdummy.exe"; fi
        else
            rm -f "$RES/svctest.cnf" "$RES/svcdummy.exe"
        fi
        if [ "${INPUTSRV:-0}" -eq 1 ]; then cp -f ../../Process/inputsrv/inputsrv.exe "$RES/"; else rm -f "$RES/inputsrv.exe"; fi
        if [ "${NETSRV:-0}" -eq 1 ]; then
            cp -f ../../Process/netsrv/netsrv.exe "$RES/"
        else
            rm -f "$RES/netsrv.exe"
        fi
    else
        rm -f "$RES/svcmgr.exe" "$RES/services.cnf" "$RES/svctest.cnf" "$RES/svcdummy.exe" "$RES/inputsrv.exe" "$RES/netsrv.exe"
    fi
    if [ "${SRVTEST:-0}" -eq 1 ]; then
        cp -f ../../Process/fssrv/fssrv.exe     "$RES/"
        cp -f ../../Process/srvtest/srvtest.exe "$RES/"
    else
        rm -f "$RES/fssrv.exe" "$RES/srvtest.exe"
    fi
    if [ "${IPCTEST:-0}" -eq 1 ]; then
        cp -f ../../Process/ipcsrv/ipcsrv.exe   "$RES/"
        cp -f ../../Process/ipctest/ipctest.exe "$RES/"
    else
        rm -f "$RES/ipcsrv.exe" "$RES/ipctest.exe"
    fi
    if [ "${NETSRV:-0}" -eq 1 ] || [ "${NETTEST:-0}" -eq 1 ]; then
        cp -f ../../Process/nettest/nettest.exe "$RES/"
    else
        rm -f "$RES/nettest.exe"
    fi
    if [ "${NO_AUDIO:-0}" -eq 0 ]; then
        cp -f ../../Process/DeodhaiAudio/deoaud.exe   ../../Resources/resources/
        cp -f ../../Process/AudioPlayer/audplr.exe    ../../Resources/resources/
    fi
    if [ "${NO_NETWORK:-0}" -eq 0 ]; then
        cp -f ../../Process/ping/ping.exe             ../../Resources/resources/
        cp -f ../../Process/udpecho/udpecho.exe       ../../Resources/resources/
        cp -f ../../Process/route/route.exe           ../../Resources/resources/
        cp -f ../../Process/iptables/iptable.exe      ../../Resources/resources/
        cp -f ../../Process/NETMngr/netmngr.exe       ../../Resources/resources/
        cp -f ../../Process/dig/dig.exe               ../../Resources/resources/
        cp -f ../../Process/nslook/nslook.exe         ../../Resources/resources/
        cp -f ../../Process/Telnet/telnet.exe         ../../Resources/resources/
        cp -f ../../Process/Finger/finger.exe         ../../Resources/resources/
        cp -f ../../Process/Gopher/gopher.exe         ../../Resources/resources/
        cp -f ../../Process/Ntpd/ntpd.exe             ../../Resources/resources/
        cp -f ../../Process/http/curl.exe             ../../Resources/resources/
        cp -f ../../Process/wifictl/wifictl.exe       ../../Resources/resources/
        if [ "${NO_NETSURF:-0}" -eq 0 ]; then
            ( cd ../../Process/NetSurf && make clean && make llvm )
            cp -f ../../Process/NetSurf/netsurf.exe   ../../Resources/resources/
        fi
    fi
fi

printf "${STY_GREEN}[llvm] AArch64 LLVM/Clang build complete.${STY_RST}\n"
