#! This script is not meant for execution, so no need for execution permission or shebang.


# color's for terminal beautification, what ? you dont like color or smt?
STY_RED='\e[31m'
STY_GREEN='\e[32m'
STY_YELLOW='\e[33m'
STY_BLUE='\e[34m'
STY_PURPLE='\e[35m'
STY_CYAN='\e[36m'

# in case we might need this 
STY_BOLD='\e[1m'
STY_FAINT='\e[2m'
STY_SLANT='\e[3m'
STY_UNDERLINE='\e[4m'
STY_BLINK='\e[5m'
STY_INVERT='\e[7m'
STY_RST='\e[00m'

# array to track files for clean up
declare -a TEMP_CLEANUP_FILES=()

