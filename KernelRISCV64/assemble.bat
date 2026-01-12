@echo off
clang.exe --target=riscv64-pc-windows-msvc -march=rv64imac -c -o %1 %2
if %errorlevel% neq 0 exit /b %errorlevel%
exit /b 0
