# _*_ coding:utf-8 _*_
"""
@Version  : 2.0.0
@Time     : 2025/10/22
@Author   : DuYu (@Duyu09, qluduyu09@163.com)
@File     : main.py
@Describe : 音频嵌入播放器(PAEP软件)编译生成环境主程序python代码 v2.0.0版本。
@Copyright: Copyright © 2023~2024 DuYu (No.11250717), Lanzhou Jiaotong University.
@Note     : 基于GCC，PyAV。
"""
import os
import av
import sys
import uuid
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
    print("--------------------------------------------------")
    file_name = os.path.abspath(input("[请输入媒体文件路径]>").replace('"', ''))
    audio_name = input("[请输入媒体文件描述]>")
    print("--------------------------------------------------")
    byte_list = "    .byte " + ", ".join([str(byte) for byte in audio_name.encode('gbk')]) + "\n"
    return file_name, audio_name, byte_list

def convert_to_wav(input_path: str, output_path: str):
    input_container = av.open(input_path)  # 打开输入媒体文件
    audio_stream = next((s for s in input_container.streams if s.type == 'audio'), None)  # 找到第一个音频流
    if audio_stream is None:
        raise ValueError("输入文件中未找到音频流！")
    src_sample_rate = audio_stream.rate or 44100
    if src_sample_rate > 46050:  # 根据规则选择目标采样率
        dst_sample_rate = 48000
    else:
        dst_sample_rate = 44100
    output_container = av.open(output_path, mode='w', format='wav')  # 打开输出 WAV 文件容器
    out_stream = output_container.add_stream("pcm_s16le", rate=dst_sample_rate)  # 新建一个输出音频流：16bit PCM、目标采样率
    resampler = av.audio.resampler.AudioResampler(
        format='s16',
        layout='stereo',  # 默认输出立体声（可根据需要修改）
        rate=dst_sample_rate
    )
    for packet in input_container.demux(audio_stream):  # 开始处理音频帧
        for frame in packet.decode():
            new_frames = resampler.resample(frame)  # 将原始帧重采样为目标格式
            if not new_frames:  # resample 可能返回一个 AudioFrame 或 list[AudioFrame]
                continue
            if not isinstance(new_frames, list):
                new_frames = [new_frames]
            for new_frame in new_frames:  # 依次写入每个重采样后的帧
                for packet in out_stream.encode(new_frame):
                    output_container.mux(packet)
    for packet in out_stream.encode(None):  # 刷新编码器缓冲区
        output_container.mux(packet)
    input_container.close()  # 关闭输入容器
    output_container.close()  # 关闭输出容器


def transcode(input_fullname, temp_directory, guid):
    print("[正在解码]")
    if not os.path.exists(input_fullname):
        error_handle("输入文件不存在，解码失败。")
    output_basename = guid + "_" + os.path.basename(input_fullname) + ".wav"
    output_fullname = os.path.join(temp_directory, output_basename)
    convert_to_wav(input_fullname, output_fullname)
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


def generate_executable(temp_directory, cpp_o_fullname, asm_fullname, exe_fullname, ld_path="ld.exe", as_path="as.exe", upx_path="upx.exe"):
    cpp_o_fullname = os.path.abspath(cpp_o_fullname)
    ld_path = os.path.abspath(ld_path)
    if not os.path.exists(asm_fullname):
        error_handle("音频数据汇编代码不存在。无法继续进行汇编与链接。")
    asm_out_basename = os.path.basename(asm_fullname) + ".o"
    asm_out_fullname = os.path.join(temp_directory, asm_out_basename)
    print("[正在汇编音频数据]")
    cmd_line01 = f'"{as_path}" --no-warn -c "{asm_fullname}" -o "{asm_out_fullname}"'
    subprocess.call(cmd_line01, cwd=os.path.dirname(as_path))
    print("[正在链接]")
    lib_path = os.path.join(os.path.dirname(ld_path), "lib")
    cmd_line02 = f'"{ld_path}" -L"{lib_path}" -lstdc++ -flto -fno-use-linker-plugin "{asm_out_fullname}" "{cpp_o_fullname}" -lwinmm -static -o "{exe_fullname}"'
    subprocess.call(cmd_line02, cwd=os.path.dirname(ld_path))
    if not os.path.exists(exe_fullname):
        error_handle("编译或链接时出现错误，生成EXE失败。")
    print("[已成功编译]")
    print("[正在使用UPX进行可执行文件压缩]")
    cmd_line03 = f'"{upx_path}" --best -9 --lzma "{exe_fullname}"'
    subprocess.call(cmd_line03, cwd=os.path.dirname(upx_path))
    if not os.path.exists(exe_fullname):
        error_handle("UPX压缩可执行文件时出现错误，生成EXE失败。")
    return exe_fullname, cpp_o_fullname, asm_out_fullname


# 这个函数是按只适于Windows平台的规则写的。
def check_running_environment():
    print("[检测运行环境]")
    temp_directory = tempfile.gettempdir()
    base_dir = os.path.dirname(os.path.abspath(__file__))
    cpp_o_fullname = os.path.join(base_dir, "template_cpp.o")
    as_path = os.path.join(base_dir, "minimal-mingw-linker\\as.exe")
    ld_path = os.path.join(base_dir, "minimal-mingw-linker\\g++.exe")
    upx_path = os.path.join(base_dir, "upx.exe")
    if (cpp_o_fullname is None) or (not os.path.exists(cpp_o_fullname)):
        error_handle("错误！预编译音乐播放器核心组件丢失。请重新安装本软件。")
    if (as_path is None) or (not os.path.exists(as_path)):
        error_handle("错误：未检测到汇编器。")
    if (ld_path is None) or (not os.path.exists(ld_path)):
        error_handle("错误：未检测到链接器。")
    if (upx_path is None) or (not os.path.exists(upx_path)):
        error_handle("错误：未检测到UPX压缩器。")
    print(f"[预编译播放器：{cpp_o_fullname}]")
    print(f"[汇编器：{as_path}]")
    print(f"[链接器：{ld_path}]")
    print(f"[UPX压缩器：{upx_path}]")
    print(f"[临时文件目录：{temp_directory}]")
    
    return cpp_o_fullname, as_path, ld_path, upx_path, temp_directory


def show_copyright():
    print("--------------------------------------------------")
    print()
    print("/███████   /██████  /████████ /███████ ")
    print("| ██__  ██ /██__  ██| ██_____/| ██__  ██")
    print("| ██  \\ ██| ██  \\ ██| ██      | ██  \\ ██")
    print("| ███████/| ████████| █████   | ███████/")
    print("| ██____/ | ██__  ██| ██__/   | ██____/ ")
    print("| ██      | ██  | ██| ██      | ██      ")
    print("| ██      | ██  | ██| ████████| ██      ")
    print("|__/      |__/  |__/|________/|__/      ")
    print()
    print("[音频嵌入播放器程序(PAEP)编译软件 v2.0]")
    print("[Copyright © 2023~2024 DuYu (@Duyu09, qluduyu09@163.com)], Lanzhou Jiaotong University]")
    print("[构建日期：Oct. 21st, 2025]")
    print("--------------------------------------------------")


if __name__ == '__main__':
    show_copyright()
    asm_tail_filename = 'emb_asm.s'
    guid = str(uuid.uuid4())
    cpp_o_fullname, as_path, ld_path, upx_path, temp_directory = check_running_environment()
    file_name, audio_name, byte_list = get_audio_information()
    wave_fullname = transcode(file_name, temp_directory, guid)
    file_size = os.path.getsize(wave_fullname)
    file_name_insert = wave_fullname.replace('\\', '\\\\')
    asm_fullname = generate_asm_code(temp_directory, guid, asm_tail_filename, file_name_insert, file_size, byte_list)
    exe_fullname = os.path.splitext(file_name)[0] + ".exe"
    real_exe_fullname, cpp_o_fullname, asm_out_fullname = generate_executable(temp_directory, cpp_o_fullname, asm_fullname, exe_fullname, ld_path, as_path, upx_path)
    del_file_list = [wave_fullname, asm_fullname, asm_out_fullname]
    clear_environment(del_file_list)
    print(f"[已生成可执行文件：{real_exe_fullname}]")
    os.system("pause")

# End of source code file of PAEP.
