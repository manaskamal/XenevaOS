#! This script is not meant for execution, so no need for execution permission or shebang.

function v(){
      echo -e "${STY_YELLOW}${STY_BOLD}##############################################################################################${STY}"
      echo -e "${STY_BLUE}[$0]: Next command:${STY_RST}"
      echo -e "${STY_GREEN}$*${STY_RST}"
      local execute=true
      if $ask;then
        while true;do
            echo -e "${STY_BLUE}Execute? ${STY_RST}"
            echo "  y = Yes"
            echo "  e = Exit now"
            echo "  s = Skip this command (NOT recommended - your setup might not work correctly)"
            echo "  yesforall = Yes and don't ask again; NOT recommended unless you really sure 🫠"
            local p; read -p "====> " p
            case $p in
                [yY]) echo -e "${STY_BLUE}OK, executing...${STY_RST}" ;break ;;
                [eE]) echo -e "${STY_BLUE}Exiting...${STY_RST}" ;exit ;break ;;
                [sS]) echo -e "${STY_BLUE}Alright, skipping this one...${STY_RST}" ;execute=false ;break ;;
                "yesforall") echo -e "${STY_BLUE}Alright, won't ask again. Executing...${STY_RST}"; ask=false ;break ;;
                *) echo -e "${STY_RED}Please enter [y/e/s/yesforall].${STY_RST}";;
            esac
        done
      fi
      if $execute;then x "$@";else
        echo -e "${STY_YELLOW}[$0]: Skipped \"$*\"${STY_RST}"
      fi
}
function x(){
    if "$@";then local cmdstatus=0;else local cmdstatus=1;fi
    while [ $cmdstatus == 1 ] ;do
        echo -e "${STY_RED}[$0]: Command \"${STY_GREEN}$*${STY_RED}\" has failed."
        echo -e "You may need to resolve the problem manually BEFORE repeating this command."
        echo "  r = Repeat this command (DEFAULT)"
        echo "  e = Exit now"
        echo "  i = Ignore this error and continue (your setup might not work correctly)"
        local p; read -p " [R/e/i]: " p
        case $p in
              [iI]) echo -e "${STY_BLUE}Alright, ignore and continue...${STY_RST}";cmdstatus=2;;
              [eE]) echo -e "${STY_BLUE}Alright, will exit.${STY_RST}";break;;
              *) echo -e "${STY_BLUE}OK, repeating...${STY_RST}"
                 if "$@";then cmdstatus=0;else cmdstatus=1;fi
             ;;
        esac
    done
    case $cmdstatus in
        0) echo -e "${STY_BLUE}[$0]: Command \"${STY_GREEN}$*${STY_BLUE}\" finished.${STY_RST}";;
        1) echo -e "${STY_RED}[$0]: Command \"${STY_GREEN}$*${STY_RED}\" has failed. Exiting...${STY_RST}";exit 1;;
        2) echo -e "${STY_RED}[$0]: Command \"${STY_GREEN}$*${STY_RED}\" has failed but ignored by user.${STY_RST}";;
    esac
}

# check for if the user is dumb enough to run the script using sudo privelages
function prevent_privelage_run(){
    case $(whoami) in
        root) echo -e "${STY_RED}[$0]: This script is NOT to be executed with sudo or as root. Aborting...${STY_RST}";exit 1;;
    esac
}

# simple function to pause the script and ask if user want to proceed
function pause(){
    if [ ! "$ask" == "false" ];then
        printf "${STY_FAINT}${STY_SLANT}"
        local p; read -p "(Ctrl-C to abort, Enter to proceed)" p
        printf "${STY_RST}"
    fi
}

# initialize a sudo session to install dependencies
function sudo_session(){
    # check for sudo
    if ! command -v sudo > /dev/null 2>&1; then
        printf "${STY_RED}sudo unavilable !${STY_RST}\n"
        return 0
    fi

    # skip if repeated
    if [[ -n "$SUDO_PID" ]] && kill -0 "$SUDO_PID" 2>/dev/null; then
        printf "${STY_YELLOW}sudo session already running aborting... ${STY_RST}\n"
        return 0
    fi

    echo -e "${STY_CYAN}[$0]: Requesting sudo privileges for installation...${STY_RST}"
    if ! sudo true; then
        echo -e "${STY_RED}[$0]: Failed to obtain sudo privileges. Aborting...${STY_RST}"
        exit 1
    fi
    (
        while true; do
            sleep 60
            sudo true 2>/dev/null || exit 0
        done
    ) &
    SUDO_PID=$!

    echo -e "${STY_GREEN}[$0]: Sudo session initialized and will be kept alive (PID: $SUDO_PID)${STY_RST}"
}

# stop the sudo session in background
function sudo_stop(){
    if [[ -n "$SUDO_PID" ]] && kill -0 "$SUDO_PID" 2>/dev/null; then
        kill "$SUDO_PID" 2>/dev/null || true
        wait "$SUDO_PID" 2>/dev/null || true
        SUDO_PID=""
    fi
}

# Help output for the scipt and information on how to use it
function show_help(){
    printf "${STY_CYAN}
Script for checking dependency and ruuning XenevaOS with llvm.

${STY_UNDERLINE}Usage:${STY_RST}${STY_CYAN} $0 [OPTION] <sub-option>

${STY_UNDERLINE}Options:${STY_RST}${STY_CYAN}
-h, --help, help          print help information.

-d, --deps, deps          check for dependencies and installs them.
    sub-option:
        --llvm, llvm      install dependencis for llvm build.
        --gcc, gcc        install dependencies for gcc build.

--llvm, llvm              compile the XenevaOS for arm(for now) using llvm toolchain (needs llvm deps)
--gcc, gcc                compile the XenevaOS for arm(for now) using gcc toolchain (needs gcc deps)
	both gcc and llvm supports sub option for:
		--build, build	  builds initrd2 image for XenevaOS


${STY_BOLD}${STY_CYAN}Access https://github.com/manaskamal/XenevaOS/blob/master/Docs/Index.md${STY_RST} ${STY_BOLD}${STY_CYAN}for documentation about XenevaOS.${STY_RST}
"
}