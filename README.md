<p align="center">
  <br>
  <img alt="paep_logo" src="https://github.com/duyu09/PCM-Wave-Audio-Embedding-Player/assets/92843163/acaf9ad1-b212-4b1f-b520-199416f3bbf1" style="width:20%;">
</p>
<br>

# PCM Wave Audio Embedding Player

音频嵌入播放器(PAEP)编译生成环境

## 软件简介

- 软件中文名称：音频嵌入播放器(PAEP)编译生成环境
- 软件英文名称：PCM Wave Audio Embedding Player Compilation and Generation Environment. (PAEP)
- 截至目前的最新版本：v1.0.0
- 软件运行依赖的环境：gcc编译器 (g++.exe)，FFmpeg解码器 (ffmpeg.exe)
- 软件运行平台：`Windows NT` (Win7以上，不跨平台)
- 简介：本软件功能是：将音频(或视频中的音频)“打包封装”为可执行文件，不依赖任何外部播放器即可播放音频。
- 原理介绍：使用`ffmpeg`，将媒体文件(目前主流格式的音频与视频)解码，根据文件的信息以及用户的相关设置，生成汇编代码，并使用`g++`编译器，将音频的PCM脉冲数据与汇编代码共同编译为机器码文件(.o文件)，然后将音频播放器cpp代码也编译为机器码文件，最后进行链接，生成可执行文件。

<img alt="arch_of_paep_v1 0" src="https://github.com/duyu09/PCM-Wave-Audio-Embedding-Player/assets/92843163/6a375ca3-45b9-4508-83e2-409b6fbfaf7c" style="width:47%;">

## 著作权声明

- PAEP Environment: Copyright © 2023~2024 DuYu (@Duyu09, qluduyu09@163.com), Faculty of Computer Science & Technology, Qilu University of Technology (Shandong Academy of Sciences).
- 按照相关开源协议，使用了开源软件FFMPEG以及GCC。
- 本软件的Logo由百度的文心一言生成，后期有修改。特此声明。

## 构建说明

- Python版本要求：Python>=3.8.
- 平台要求：`Windows NT` 不跨平台；硬件不限，AMD架构与ARM架构等均可。经试验，Linux环境中使用Wine兼容层可以很好地构建和运行PAEP。
- 本软件唯一的依赖库是打包工具PyInstaller，安装命令：`pip install pyinstaller`
- 运行 `build_pure.bat` 即可构建纯净版的PAEP Environment软件。
- 请下载ffmpeg以及MinGW64编译器，将ffmpeg.exe置于项目根目录，将 `MinGW64/MinGW64/...` 置于项目根目录 (注意有两层MinGW64，不要破坏目录结构)，运行 `build_with_ffmpeg.bat` 、`build_with_mingw64.bat` 、`build_full.bat` 可分别打包构建带有ffmpeg解码器的、带有MinGW64编译器的与全量版本的PAEP Environment软件。

## 运行说明

- 我们发行了PAEP ENVIRONMENT的可执行文件，也可按照上述方法自行打包构建。发行地址：
<br> https://github.com/duyu09/PCM-Wave-Audio-Embedding-Player/releases <br>
- 建议：运行纯净版的PAEP Environment软件 (即：`paep_pure_v1.0.0_windows_x86-64.exe`) 需要用户自行在计算机上安装FFmpeg软件以及MinGW64编译器，并配置它们的可执行文件到PATH环境变量中，以确保PAEP能够访问到这些软件。
- 不建议：不建议运行ffmpeg或MinGW64嵌入版以及全量版。用户可以直接运行，但文件体积过大，并且启动缓慢。
- 生成的音频播放器不依赖任何环境便可以运行播放音频。

| FFmpeg官方下载地址 | MinGW64编译器官方下载地址 |
| ----- | ----- |
| https://ffmpeg.org/download.html#build-windows | https://www.mingw-w64.org/downloads |

## 软件更新日志

- Update on Jan. 15th, 2024
- - 创建开源代码仓库，提交PAEP软件v1.0版本的全部代码。
 
## 友情链接

- 齐鲁工业大学(山东省科学院) 计算机科学与技术学部 http://jsxb.scsc.cn/
- 山东省计算中心 (国家超级计算济南中心) https://www.nsccjn.cn/
- FFMPEG官方网站 https://ffmpeg.org/
- MinGW-W64官方网站 https://www.mingw-w64.org/
- DuYu的个人网站 https://www.duyu09.site/
- DuYu的GitHub网站 https://github.com/duyu09/

## 访客统计

<div><b>Number of Total Visits (All of Duyu09's GitHub Projects): </b><br><img src="https://profile-counter.glitch.me/duyu09/count.svg" /></div> 

<div><b>Number of Total Visits (PAEP): </b><br><img src="https://profile-counter.glitch.me/duyu09-PAEP/count.svg" /></div> 

