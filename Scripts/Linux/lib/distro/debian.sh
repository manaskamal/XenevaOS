#! This script is not meant for execution, so no need for execution permission or shebang.

if ! command -v apt >/dev/null 2>&1; then
  printf "${STY_RED}[$0]: apt not found, are you sure you are on debian based system (you sussy baka!) ${STY_RST}\n"
  exit 1
fi

v sudo apt update
v sudo apt upgrade
v sudo apt install dosfstools
