/*
Copyright (c) 2023~2025 DuYu (@Duyu09, qluduyu09@163.com), Lanzhou Jiaotong University.
CPP Source code of Duyu PCM-WAVE-AUDIO-EMBEDDING PLAYER (PAEP) v2.0.0.
This file is an important part of PAEP.
*/

#include <bits/stdc++.h>
#include <windows.h>
#include <mmsystem.h>
#include <algorithm>

using namespace std;

#define NUM_BUFFERS 4
#define BUFFER_SIZE 40960  // 减小单个缓冲区大小，增加响应性

// 全局变量
short buffers[NUM_BUFFERS][BUFFER_SIZE];
WAVEHDR waveHeaders[NUM_BUFFERS];
volatile bool bufferInUse[NUM_BUFFERS] = {false};
char *da_ptr = nullptr;
char *da_tail_ptr = nullptr;
volatile bool isPlaying = false;
HWAVEOUT global_hWaveOut = nullptr;

extern "C" {
	extern const char embedded_data[];
	extern const size_t embedded_data_size;
	extern const char embedded_audio_name[];
}

typedef struct {
	char chunkID[4];
	unsigned int chunkSize;
	char format[4];
	char subchunk1ID[4];
	unsigned int subchunk1Size;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;
	char subchunk2ID[4];
	unsigned int subchunk2Size;
} WavHeader;

// 安全的回调函数
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	if (uMsg == WOM_DONE && isPlaying) {
		WAVEHDR* hdr = (WAVEHDR*)dwParam1;
		if (hdr && (hdr->dwFlags & WHDR_PREPARED)) {
			int bufferIndex = (int)hdr->dwUser;
			if (bufferIndex >= 0 && bufferIndex < NUM_BUFFERS) {
				bufferInUse[bufferIndex] = false;
			}
		}
	}
}

void printDeviceInfo() {
	printf("Audio Devices Information:\n");
	UINT numDevices = waveOutGetNumDevs();
	printf("Number of audio devices available: %d\n", numDevices);
	
	for (int i = 0; i <= numDevices - 1; i++) {
		WAVEOUTCAPS waveOutCaps;
		MMRESULT result = waveOutGetDevCaps(i, &waveOutCaps, sizeof(WAVEOUTCAPS));
		if (result == MMSYSERR_NOERROR) {
			printf("Device: %s  Version: %d.%d\n", waveOutCaps.szPname, 
				HIBYTE(waveOutCaps.vDriverVersion), LOBYTE(waveOutCaps.vDriverVersion));
		} else {
			printf("Failed to get device information for device %d.\n", i);
		}
	}
	printf("\n");
}

void printAudioInfo(WavHeader wavheader) {
	printf("Audio Information:\n");
	printf("Audio name: %s\n", embedded_audio_name);
	printf("Sample rate: %d Hz\n", wavheader.sampleRate);
	printf("Bit depth: %d bit/sample\n", wavheader.bitsPerSample);
	printf("Number of channels: %d channel(s)\n", wavheader.numChannels);
	printf("\n");
}

void printCopyrightInfo() {
	printf("PCM-AUDIO-EMBEDDING PLAYER v2.0\n\n");
	printf("Copyright (c) 2023~2025 DuYu (@Duyu09), Lanzhou Jiaotong University.\n");
	printf("\n");
}

// 安全的数据拷贝函数
int safe_memcpy(short *dst, char *src, int size, const char *tail_ptr) {
	if (!dst || !src || !tail_ptr) return 0;
	
	ptrdiff_t diff_d = tail_ptr - src;
	if (diff_d <= 0) return 0;
	
	int bytes_to_copy = min((int)diff_d, size);
	bytes_to_copy = min(bytes_to_copy, BUFFER_SIZE);
	
	// 确保不会拷贝超出数据范围
	if (bytes_to_copy > 0) {
		memcpy(dst, src, bytes_to_copy);
	}
	
	return bytes_to_copy;
}

// 安全的清理函数
void safeCleanup(HWAVEOUT hWaveOut) {
	if (hWaveOut) {
		// 先停止播放
		waveOutReset(hWaveOut);
		
		// 清理所有缓冲区头
		for (int i = 0; i < NUM_BUFFERS; i++) {
			if (waveHeaders[i].dwFlags & WHDR_PREPARED) {
				waveOutUnprepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
			}
		}
		
		// 关闭设备
		waveOutClose(hWaveOut);
	}
}

int main(int argc, char *argv[]) {
	printCopyrightInfo();
	printDeviceInfo();
	
	// 安全检查嵌入式数据
	if (embedded_data == nullptr || embedded_data_size == 0) {
		printf("ERROR: FAILED TO GET AUDIO DATA OR DATA IS EMPTY!\n");
		return 1;
	}
	
	int arrlen = embedded_data_size;
	char *dataArr = (char *)embedded_data;
	da_ptr = dataArr;
	da_tail_ptr = da_ptr + arrlen - 1;
	
	// 检查WAV文件头
	if (arrlen < sizeof(WavHeader)) {
		printf("ERROR: AUDIO DATA TOO SMALL TO CONTAIN VALID WAV HEADER!\n");
		return 1;
	}
	
	WavHeader header;
	memcpy(&header, da_ptr, sizeof(header));
	da_ptr += sizeof(header);
	
	printAudioInfo(header);
	
	// 验证WAV格式
	if (memcmp(header.chunkID, "RIFF", 4) != 0 ||
		memcmp(header.format, "WAVE", 4) != 0 ||
		memcmp(header.subchunk1ID, "fmt ", 4) != 0) {
		printf("ERROR: UNKNOWN OR INVALID WAV FORMAT!\n");
		return EXIT_FAILURE;
	}
	
	// 只支持16位PCM
	if (header.bitsPerSample != 16) {
		printf("ERROR: Sorry, this version only supports 16-bit PCM wave audio.\n");
		return EXIT_FAILURE;
	}
	
	// 初始化音频设备
	HWAVEOUT hWaveOut;
	WAVEFORMATEX wfx;
	
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = header.numChannels;
	wfx.nSamplesPerSec = header.sampleRate;
	wfx.nAvgBytesPerSec = header.byteRate;
	wfx.nBlockAlign = header.blockAlign;
	wfx.wBitsPerSample = header.bitsPerSample;
	wfx.cbSize = 0;
	
	MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 
		(DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
	if (result != MMSYSERR_NOERROR) {
		printf("ERROR: CANNOT OPEN AUDIO DEVICE! Error code: %d\n", result);
		return EXIT_FAILURE;
	}
	
	global_hWaveOut = hWaveOut;
	isPlaying = true;
	
	// 初始化所有缓冲区
	printf("Initializing %d audio buffers...\n", NUM_BUFFERS);
	for (int i = 0; i < NUM_BUFFERS; i++) {
		memset(&waveHeaders[i], 0, sizeof(WAVEHDR));
		waveHeaders[i].lpData = (LPSTR)buffers[i];
		waveHeaders[i].dwBufferLength = 0;
		waveHeaders[i].dwUser = i;  // 存储缓冲区索引
		
		result = waveOutPrepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
		if (result != MMSYSERR_NOERROR) {
			printf("ERROR: Failed to prepare buffer header %d. Error: %d\n", i, result);
			safeCleanup(hWaveOut);
			return EXIT_FAILURE;
		}
		bufferInUse[i] = false;
	}
	
	printf("PLAYING...\n");
	printf("Press Ctrl+C to stop playback.\n\n");
	
	int frameCounter = 1;
	bool hasData = true;
	
	// 主播放循环
	while (hasData && isPlaying) {
		bool submittedData = false;
		
		for (int i = 0; i < NUM_BUFFERS && hasData; i++) {
			if (!bufferInUse[i]) {
				// 填充可用缓冲区
				int bytes_copied = safe_memcpy(buffers[i], da_ptr, BUFFER_SIZE, da_tail_ptr);
				
				if (bytes_copied > 0) {
					waveHeaders[i].dwBufferLength = bytes_copied;
					
					result = waveOutWrite(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
					if (result == MMSYSERR_NOERROR) {
						bufferInUse[i] = true;
						da_ptr += bytes_copied;
						printf("Frame %d (buffer %d): %d bytes\r", frameCounter++, i, bytes_copied);
						fflush(stdout);
						submittedData = true;
					} else {
						printf("ERROR: Failed to write to buffer %d. Error: %d\n", i, result);
						bufferInUse[i] = false;
					}
				} else {
					hasData = false;  // 没有更多数据
				}
			}
		}
		
		if (!submittedData) {
			// 没有提交新数据，短暂休眠避免CPU占用过高
			Sleep(1);
		}
		
		// 检查是否所有数据都已播放完毕
		if (!hasData) {
			bool stillPlaying = false;
			for (int i = 0; i < NUM_BUFFERS; i++) {
				if (bufferInUse[i]) {
					stillPlaying = true;
					break;
				}
			}
			if (!stillPlaying) {
				break;  // 所有缓冲区都已播放完毕且无新数据
			}
		}
	}
	
	printf("\n\nPlayback finished.\n");
	
	// 等待所有缓冲区播放完毕
	printf("Waiting for buffers to finish...\n");
	bool stillPlaying;
	int waitCount = 0;
	const int MAX_WAIT = 500; // 最多等待5秒
	
	do {
		stillPlaying = false;
		for (int i = 0; i < NUM_BUFFERS; i++) {
			if (bufferInUse[i]) {
				stillPlaying = true;
				break;
			}
		}
		if (stillPlaying) {
			Sleep(10);
			waitCount++;
			if (waitCount > MAX_WAIT) {
				printf("Timeout waiting for playback to finish.\n");
				break;
			}
		}
	} while (stillPlaying);
	
	// 安全清理
	isPlaying = false;
	safeCleanup(hWaveOut);
	global_hWaveOut = nullptr;
	
	printf("Playback completed successfully.\n");
	return 0;
}

