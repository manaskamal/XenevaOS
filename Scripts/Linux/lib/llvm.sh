#! This script is not meant for execution, so no need for execution permission or shebang.


printf "${STY_CYAN}compiling Xeneva for arm using llvm.${STY_RST}\n"
cd ../../BootAA64
make clean
make llvm

cd ../KernelAA64
make clean
make llvm

cd ..