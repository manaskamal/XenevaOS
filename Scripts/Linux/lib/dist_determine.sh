#! This script is not meant for execution, so no need for execution permission or shebang.

function print_os_group_id(){
    printf "${STY_CYAN}"
    printf "===INFO===\n"
    printf "Detected OS_DISTRO_ID: ${OS_DISTRO_ID}\n"
    printf "Detected OS_DISTRO_ID_LIKE: ${OS_DISTRO_ID_LIKE}\n"
    printf "Determined OS_GROUP_ID: ${OS_GROUP_ID}\n"
    printf "==========\n\n"
    printf "${STY_RST}"
}
function print_os_group_id_alike(){
    printf "${STY_YELLOW}"
    printf "===WARNING===\n"
    printf "Your OS_GROUP_ID has been determined by \"alike\" match.\n"
    printf "Ideally, it should also work for your distro.\n"
    printf "Still, there is a chance that it not works as expected or even fails.\n"
    printf "Proceed only at your own risk.\n"
    printf "=============\n\n"
    printf "${STY_RST}"
}
function print_os_group_id_unsupported(){
    printf "${STY_RED}"
    printf "===CAUTION===\n"
    printf "\"--via-nix\" is forcely specified \n"
    printf "as the only way to try to install on your distro.\n"
    printf "It is still experimental.\n"
    printf "Some functionalities are missing.\n"
    printf "It may also behave unexpectedly.\n"
    printf "Proceed only at your own risk.\n"
    printf "=============\n\n"
    printf "${STY_RST}"
    sleep 3
}
function print_os_group_id_fallback(){
    printf "${STY_RED}"
    printf "===CAUTION===\n"
    printf "Distro not recognized, determined as fallback.\n"
    printf "=============\n\n"
    printf "${STY_RST}"
}
function print_os_group_id_architecture(){
    printf "${STY_RED}"
    printf "===CAUTION===\n"
    printf "Detected machine architecture: ${MACHINE_ARCH}\n"
    printf "This script only supports x86_64.\n"
    printf "It is very likely to fail when installing dependencies on your machine.\n"
    printf "=============\n\n"
    printf "${STY_RST}"
}

#Detecting distro
OS_RELEASE_FILE_CUSTOM="${REPO_ROOT}/os-release"
if test -f "${OS_RELEASE_FILE_CUSTOM}"; then
    printf "${STY_YELLOW}Warning: using custom os-release file \"${OS_RELEASE_FILE_CUSTOM}\".${STY_RST}\n"
    OS_RELEASE_FILE="${OS_RELEASE_FILE_CUSTOM}"
elif test -f /etc/os-release; then
    OS_RELEASE_FILE=/etc/os-release
else
    printf "${STY_RED}/etc/os-release does not exist, aborting...${STY_RST}\n" ; exit 1
fi
export OS_DISTRO_ID=$(awk -F'=' '/^ID=/ { gsub(/["\x27]/,"",$2); print tolower($2) }' ${OS_RELEASE_FILE} 2> /dev/null)
export OS_DISTRO_ID_LIKE=$(awk -F'=' '/^ID_LIKE=/ { gsub(/["\x27]/,"",$2); print tolower($2) }' ${OS_RELEASE_FILE} 2> /dev/null)


# Determine distro
if [[ "$OS_DISTRO_ID" =~ ^(arch|endeavouros|cachyos)$ ]]; then
    OS_GROUP_ID="arch"
    print_os_group_id_functions=(print_os_group_id)
elif [[ "$OS_DISTRO_ID_LIKE" == "arch" ]]; then
    OS_GROUP_ID="arch"
    print_os_group_id_functions=(print_os_group_id{,_alike})
elif [[ "$OS_DISTRO_ID" =~ ^(ubuntu|debian|linuxmint|pop|kali|elementary|zorin|deepin|mx|pureos|tails|neon|linuxmint|peppermint|bodhi|parrot|)$ ]]; then
    OS_GROUP_ID="debian"
    print_os_group_id_functions=(print_os_group_id)
elif [[ "$OS_DISTRO_ID_LIKE" == "debian" ]]; then
    OS_GROUP_ID="debian"
    print_os_group_id_functions=(print_os_group_id{,_alike})
else
    OS_GROUP_ID="fallback"
    INSTALL_VIA_NIX=true
    print_os_group_id_functions=(print_os_group_id{,_fallback,_unsupported})
fi