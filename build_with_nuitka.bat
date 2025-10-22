@echo off
:: Copyright © 2023~2025  DuYu (No. 11250717), School of Electronic and Information Engineering, Lanzhou Jiaotong University.
:: Windows Batch File of building PAEP v2.0.0.

cd /d "%~dp0"
echo [Start to build PAEP]
echo [Compiling...]
g++ -O3 -c template_cpp.cpp -o ./bin/template_cpp.o
echo [Build EXE File...]
nuitka --onefile --standalone --windows-icon-from-ico=./bin/paep_logo.ico --output-filename=paep_v2.0.0_windows_x86-64.exe --include-data-files=./bin/template_cpp.o=template_cpp.o --include-data-dir=./bin/minimal-mingw-linker=minimal-mingw-linker --include-data-file=./bin/upx.exe=upx.exe --include-data-files=./bin/minimal-mingw-linker/ld.exe=minimal-mingw-linker/ld.exe --include-data-files=./bin/minimal-mingw-linker/as.exe=minimal-mingw-linker/as.exe --include-data-files=./bin/minimal-mingw-linker/g++.exe=minimal-mingw-linker/g++.exe --show-progress --lto=yes --remove-output main.py
echo [Finish]
pause