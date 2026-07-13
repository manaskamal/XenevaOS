#! This script is not meant for execution, so no need for execution permission or shebang.

if ! command -v apt >/dev/null 2>&1; then
	printf "${STY_RED}[$0]: apt not found, are you sure you are on debian based system (you sussy baka!) ${STY_RST}\n"
	exit 1
fi

# update the system
v sudo apt update
v sudo apt upgrade

# common for both llvm and gcc
v sudo apt install make
v sudo apt install dosfstools

case $2 in 
	--llvm|llvm)
		v sudo apt install clang
		v sudo apt install lld
		v sudo apt install llvm
	;;
	--gcc|gcc)
		v sudo apt install gcc-aarch64-linux-gnu
		v sudo apt install binutils-aarch64-linux-gnu
		cd ../..
		v git clone https://github.com/vathpela/gnu-efi
	;;
	*)
		printf "${STY_RED}Plese choose from llvm and gcc. Aborting ...${STY_RST}\n"
	;;
esac
