# _*_ coding:utf-8 _*_
"""
@Version  : 1.0.0
@Time     : 2024/01/13
@Author   : DuYu (@Duyu09, qluduyu09@163.com)
@File     : main.py
@Describe : 音频嵌入播放器(PAEP软件)编译生成环境主程序python代码 v1.0.0版本。
@Copyright: Copyright © 2023~2024 DuYu (No.202103180009), Faculty of Computer Science & Technology, Qilu University of Technology (Shandong Academy of Sciences).
@Note     : 基于G++，FFmpeg。
"""
import os
import sys
import uuid
import shutil
import tempfile
import subprocess


def clear_environment(del_file_list):
    print("[清理临时文件]")
    for f in del_file_list:
        try:
            os.remove(f)
        except Exception:
            pass
    print("[临时文件清理完毕]")


def error_handle(message):
    print(f"[ERROR: {message}]")
    os.system("pause")
    sys.exit(1)


def get_audio_information():
    file_name = os.path.abspath(input("[请输入媒体文件路径]>").replace('"', ''))
    audio_name = input("[请输入媒体文件描述]>")
    byte_list = "    .byte " + ", ".join([str(byte) for byte in audio_name.encode('gbk')]) + "\n"
    return file_name, audio_name, byte_list


def transcode(input_fullname, temp_directory, guid, ffmpeg_path=r'./ffmpeg'):
    print("[正在解码]")
    if not os.path.exists(input_fullname):
        error_handle("输入文件不存在，解码失败。")
    output_basename = guid + "_" + os.path.basename(input_fullname) + ".wav"
    output_fullname = os.path.join(temp_directory, output_basename)
    commandline = f'"{ffmpeg_path}" -i "{input_fullname}" -vn -acodec pcm_s16le "{output_fullname}"'
    subprocess.call(commandline, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
    if not os.path.exists(output_fullname):
        error_handle("解码失败。")
    return output_fullname


def generate_asm_code(temp_directory, guid, asm_tail_filename, file_name_insert, file_size, byte_list):
    print("[生成音频数据的汇编代码]")
    asm_filename_with_guid = guid + "_" + asm_tail_filename
    asm_code_full_name = os.path.join(temp_directory, asm_filename_with_guid)
    with open(asm_code_full_name, 'w') as file:
        file.write('.section .data\n')
        file.write('    .global embedded_data\n')
        file.write('    .global embedded_data_size\n')
        file.write('    .global embedded_audio_name\n')
        file.write('\n')
        file.write('embedded_data:\n')
        file.write(f'    .incbin "{file_name_insert}"\n')
        file.write('\n')
        file.write('embedded_data_size:\n')
        file.write(f'    .int {file_size}\n')
        file.write('\n')
        file.write('embedded_audio_name:\n')
        file.write(byte_list)
        file.write('\n')
    return asm_code_full_name


def generate_executable(temp_directory, guid, cpp_fullname, asm_fullname, exe_fullname, gpp_path="g++"):
    cpp_out_basename = guid + "_" + os.path.basename(cpp_fullname) + ".o"
    cpp_out_fullname = os.path.join(temp_directory, cpp_out_basename)
    if not os.path.exists(asm_fullname):
        error_handle("音频数据汇编代码不存在。无法继续进行汇编与链接。")
    asm_out_basename = os.path.basename(asm_fullname) + ".o"
    asm_out_fullname = os.path.join(temp_directory, asm_out_basename)

    print("[正在编译播放器组件]")
    cmd_line01 = f'"{gpp_path}" -Wfatal-errors -c "{cpp_fullname}" -o "{cpp_out_fullname}"'
    subprocess.call(cmd_line01)
    print("[正在汇编音频数据]")
    cmd_line02 = f'"{gpp_path}" -Wfatal-errors -c "{asm_fullname}" -o "{asm_out_fullname}"'
    subprocess.call(cmd_line02)
    print("[正在链接]")
    cmd_line03 = f'"{gpp_path}" "{asm_out_fullname}" "{cpp_out_fullname}" -Wfatal-errors -finput-charset=UTF-8 -fexec-charset=gbk -lwinmm -lstdc++ -static -o "{exe_fullname}"'
    subprocess.call(cmd_line03)
    if not os.path.exists(exe_fullname):
        error_handle("编译或链接时出现错误，生成EXE失败。")
    print("[已成功编译]")
    return exe_fullname, cpp_out_fullname, asm_out_fullname


# 这个函数是按只适于Windows平台的规则写的。如考虑跨平台，需考虑“exe”。
def check_running_environment():
    print("[检测运行环境]")
    temp_directory = tempfile.gettempdir()
    base_dir = os.path.dirname(os.path.abspath(__file__))
    cpp_fullname = os.path.join(base_dir, "template_cpp.cpp")
    ffmpeg_path = os.path.join(base_dir, "ffmpeg.exe")
    if not os.path.exists(ffmpeg_path):
        ffmpeg_relative_path = shutil.which("ffmpeg.exe")
        if ffmpeg_relative_path is None:
            ffmpeg_path = None
        else:
            ffmpeg_path = os.path.abspath(ffmpeg_relative_path)
    gpp_path = os.path.join(base_dir, "MinGW64\\bin\\g++.exe")
    if not os.path.exists(gpp_path):
        gpp_relative_path = shutil.which("g++.exe")
        if gpp_relative_path is None:
            gpp_path = None
        else:
            gpp_path = os.path.abspath(gpp_relative_path)
    if (cpp_fullname is None) or (not os.path.exists(cpp_fullname)):
        error_handle("错误！音乐播放器核心CPP代码丢失，无法继续编译。请重新安装本软件。")
    if (ffmpeg_path is None) or (not os.path.exists(ffmpeg_path)):
        error_handle("错误！未检测到ffmpeg。请下载ffmpeg并将其配置到环境变量PATH中，或安装本软件的全量版本。")
    if (gpp_path is None) or (not os.path.exists(gpp_path)):
        error_handle("错误：未检测到g++编译器。请下载GCC并将其配置到环境变量PATH中，或安装本软件的全量版本。")
    print(f"[g++编译器：{gpp_path}]")
    print(f"[FFmpeg解码器：{ffmpeg_path}]")
    return cpp_fullname, gpp_path, ffmpeg_path, temp_directory


def show_copyright():
    print("[音频嵌入播放器程序(PAEP)编译软件 v1.0]")
    print("[Copyright © 2023~2024 DuYu (@Duyu09, qluduyu09@163.com)], Faculty of Computer Science & Technology, Qilu University of Technology (Shandong Academy of Sciences)]")
    print("[构建信息：Jan. 15th, 2024 by PyInstaller v6.3]")


if __name__ == '__main__':
    show_copyright()
    asm_tail_filename = 'emb_asm.s'
    guid = str(uuid.uuid4())
    cpp_fullname, gpp_path, ffmpeg_path, temp_directory = check_running_environment()
    file_name, audio_name, byte_list = get_audio_information()
    wave_fullname = transcode(file_name, temp_directory, guid, ffmpeg_path)
    file_size = os.path.getsize(wave_fullname)
    file_name_insert = wave_fullname.replace('\\', '\\\\')
    asm_fullname = generate_asm_code(temp_directory, guid, asm_tail_filename, file_name_insert, file_size, byte_list)
    exe_fullname = os.path.splitext(file_name)[0] + ".exe"
    real_exe_fullname, cpp_out_fullname, asm_out_fullname = generate_executable(temp_directory, guid, cpp_fullname,
                                                                                asm_fullname, exe_fullname, gpp_path)
    del_file_list = [wave_fullname, asm_fullname, cpp_out_fullname, asm_out_fullname]
    clear_environment(del_file_list)
    print(f"[已生成可执行文件：{real_exe_fullname}]")
    os.system("pause")

# End of source code file of PAEP.
