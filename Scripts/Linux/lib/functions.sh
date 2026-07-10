#! This script is not meant for execution, so no need for execution permission or shebang.

function prevent_privelage_run(){
    case $(whoami) in
        root) echo -e "${STY_RED}[$0]: This script is NOT to be executed with sudo or as root. Aborting...${STY_RST}";exit 1;;
    esac
}

function pause(){
  if [ ! "$ask" == "false" ];then
    printf "${STY_FAINT}${STY_SLANT}"
    local p; read -p "(Ctrl-C to abort, Enter to proceed)" p
    printf "${STY_RST}"
  fi
}

function show_help(){
    printf "${STY_CYAN}
Script for checking dependency and ruuning XenevaOS with llvm.

${STY_UNDERLINE}Usage:${STY_RST}${STY_CYAN} $0 [OPTION]

${STY_UNDERLINE}Options:${STY_RST}${STY_CYAN}
-h, --help          print help information.
-d, --deps          check for dependencies and installs them.


${STY_BOLD}${STY_CYAN}Access https://github.com/manaskamal/XenevaOS/blob/master/Docs/Index.md${STY_RST} ${STY_BOLD}${STY_CYAN}for documentation about XenevaOS.${STY_RST}
"
}