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

# update the system
v sudo pacman -Syu --noconfirm

# common for both llvm and gcc
v sudo pacman -S --needed --noconfirm make
v sudo pacman -S --needed --noconfirm dosfstools
v sudo pacman -S --needed --noconfirm mtools
v sudo pacman -S --needed --noconfirm qemu-system-aarch64
v sudo pacman -S --needed --noconfirm edk2-aarch64

case $2 in
	--llvm|llvm)
		v sudo pacman -S --needed --noconfirm clang
		v sudo pacman -S --needed --noconfirm lld
		v sudo pacman -S --needed --noconfirm llvm
	;;
	--gcc|gcc)
		v sudo pacman -S --needed --noconfirm aarch64-linux-gnu-gcc
		v sudo pacman -S --needed --noconfirm aarch64-linux-gnu-binutils
		cd ../..
		v git clone https://github.com/vathpela/gnu-efi
	;;
	*)
		printf "${STY_RED}Plese choose from llvm and gcc. Aborting ...${STY_RST}\n"
	;;
esac