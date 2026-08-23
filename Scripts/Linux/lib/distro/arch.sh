#! this script is only meant to be sourced.

# install-yay(){
#   x sudo pacman -S --needed --noconfirm base-devel
#   x git clone https://aur.archlinux.org/yay-bin.git /tmp/buildyay
#   x cd /tmp/buildyay
#   x makepkg -o
#   x makepkg -se
#   x makepkg -i --noconfirm
#   x cd ${REPO_ROOT}
#   rm -rf /tmp/buildyay
# }

if ! command -v pacman >/dev/null 2>&1; then
  printf "${STY_RED}[$0]: pacman not found, are you sure you are on arch (you sussy baka!) ${STY_RST}\n"
  exit 1
fi

# this prevents makepkg from resetting sudo
if [[ -z "${PACMAN_AUTH:-}" ]]; then
  export PACMAN_AUTH="sudo"
fi