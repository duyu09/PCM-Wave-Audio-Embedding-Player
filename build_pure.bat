@echo off
:: Copyright Statement: Copyright © 2023~2024  DuYu (No.202103180009), Faculty of Computer Science & Technology, Qilu University of Technology (Shandong Academy of Sciences).
:: Windows Batch File of building PAEP_Pure v1.0.0.

pyinstaller -i paep_logo.ico -F main.py --version-file version.txt --add-data "template_cpp.cpp;." --upx-dir UPX.exe -n paep_pure_v1.0.0_windows_x86-64.exe

pause