/*
Copyright (c) 2023~2024 DuYu (@Duyu09, qluduyu09@163.com), Faculty of Computer Science and Technology, Qilu University of Technology (Shandong Academy of Sciences).
CPP Source code of Duyu PCM-WAVE-AUDIO-EMBEDDING PLAYER (PAEP) v1.0.0.
This file is an important part of PAEP.
*/

#include <bits/stdc++.h>
#include <windows.h>
#include <mmsystem.h>
#include <algorithm>

using namespace std;
#define BUFFER_SIZE 16384000
short buffer[BUFFER_SIZE];

extern "C" {
	extern const char embedded_data[];
	extern const size_t embedded_data_size;
	extern const char embedded_audio_name[];
}

int arrlen = embedded_data_size;
char *dataArr = (char *) embedded_data;
char *da_ptr = dataArr;
char *da_tail_ptr = da_ptr + arrlen - 1;
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

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	// A callback function that executes when an audio event needs to be handled can be left blank.
}

void printDeviceInfo() {
	printf("Audio Devices Information:\n");
	UINT numDevices = waveOutGetNumDevs();
	printf("Number of audio devices available: %d\n", numDevices);
	
	for (int i = 0; i <= numDevices - 1; i++) {
		WAVEOUTCAPS waveOutCaps;
		MMRESULT result = waveOutGetDevCaps(i, &waveOutCaps, sizeof(WAVEOUTCAPS));
		if (result == MMSYSERR_NOERROR) {
			printf("Device: %s  Version: %d.%d\n", waveOutCaps.szPname, HIBYTE(waveOutCaps.vDriverVersion),
				LOBYTE(waveOutCaps.vDriverVersion));
			// More device information, such as supported formats, can be printed.
		} else {
			printf("Failed to get device information.\n");
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
	printf("PCM-AUDIO-EMBEDDING PLAYER v1.0\n\n");
	printf("Copyright (c) 2024 DuYu (@Duyu09), Faculty of Computer Science and Technology, Qilu University of Technology (Shandong Academy of Sciences).\n");
	printf("\n");
}

int def_memcpy(short *dst, char *src, int size, const char *tail_ptr) {
	int bs = 0;
	ptrdiff_t diff_d = tail_ptr - src;
	int diff = (int) diff_d;
	if (diff <= 0) return 0;
	if (BUFFER_SIZE > diff) { bs = diff; }
	else { bs = BUFFER_SIZE; }
	memcpy(dst, src, bs);
	return bs;
}

int main(int argc, char *argv[]) {
	printCopyrightInfo();
	printDeviceInfo();
	if (dataArr == nullptr) {
		printf("ERROR: FAILED GET AUDIO DATA!\n");
		return 1;
	}
	
	WavHeader header;
	memcpy(&header, da_ptr, sizeof(header));
	da_ptr += sizeof(header);
	printAudioInfo(header);
	
	if (memcmp(header.chunkID, "RIFF", 4) != 0 ||
		memcmp(header.format, "WAVE", 4) != 0 ||
		memcmp(header.subchunk1ID, "fmt ", 4) != 0)
		// memcmp(header.subchunk2ID, "data", 4) != 0)
	{
		printf("ERROR: UNKNOWN FORMAT!\n");
		return EXIT_FAILURE;
	}
	
	if (header.bitsPerSample != 16) {
		printf("ERROR: Sorry, this version of player only support 16-bit PCM wave audio.\n");
		return EXIT_FAILURE;
	}
	
	HWAVEOUT hWaveOut;
	WAVEFORMATEX wfx;
	
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = header.numChannels;
	wfx.nSamplesPerSec = header.sampleRate;
	wfx.nAvgBytesPerSec = header.byteRate;
	wfx.nBlockAlign = header.blockAlign;
	wfx.wBitsPerSample = header.bitsPerSample;
	wfx.cbSize = 0;
	
	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR) waveOutProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		//if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		printf("ERROR: CANNOT OPEN AUDIO DEVICE!\n");
		return EXIT_FAILURE;
	}
	
//	DWORD newPitch=100;
//	MMRESULT r=waveOutSetPitch(hWaveOut, newPitch);
	
	int frameCounter = 1;
	printf("PLAYING...\n");
	while (true) {
		int bs = def_memcpy(buffer, da_ptr, BUFFER_SIZE, da_tail_ptr);
		if (bs < BUFFER_SIZE) {
			bs = max(0, bs - 6144);
		}
		if (bs <= 0) {
			break;
		}
		printf("Frame %d: %d Bytes.\n", frameCounter, bs);
		++frameCounter;
		da_ptr += bs;
		WAVEHDR WaveHdr;
		WaveHdr.lpData = (LPSTR) buffer;
		WaveHdr.dwBufferLength = bs;
		WaveHdr.dwBytesRecorded = 0;
		WaveHdr.dwUser = 0;
		WaveHdr.dwFlags = 0;
		WaveHdr.dwLoops = 0;
		
		waveOutPrepareHeader(hWaveOut, &WaveHdr, sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, &WaveHdr, sizeof(WAVEHDR));
		
		while (waveOutUnprepareHeader(hWaveOut, &WaveHdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING) {
			Sleep(16);
		}
	}
	waveOutClose(hWaveOut);
	return 0;
}


