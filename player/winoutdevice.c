/*
 * An example showing how to use the Karma softsynth
 * 
 * $Id: winoutdevice.c,v 1.3 2004/01/01 16:06:19 Fredrik Ehnbom Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#include <windows.h>

#include "device.h"

#define REPLAY_NBSOUNDBUFFER  3

int m_currentBuffer = 0;
WAVEHDR m_waveHeader[REPLAY_NBSOUNDBUFFER];
HWAVEOUT m_hWaveOut;
BOOL closing = FALSE;
short soundBuffer[BUFFERSIZE * 2 * REPLAY_NBSOUNDBUFFER];

static __declspec(align(32)) int bufferL[BUFFERSIZE];
static __declspec(align(32)) int bufferR[BUFFERSIZE];


// Internal WaveOut API callback function. We just call our sound handler ("playNextBuffer")
void	fillNextBuffer() {
	int pos;
	int i;

	if (closing) return;
	// check if the buffer is already prepared (should not !)
	if (m_waveHeader[m_currentBuffer].dwFlags&WHDR_PREPARED)
		waveOutUnprepareHeader(m_hWaveOut,&m_waveHeader[m_currentBuffer],sizeof(WAVEHDR));

	memset(&bufferL, 0, BUFFERSIZE * sizeof(int));
	memset(&bufferR, 0, BUFFERSIZE * sizeof(int));

	karma_process(ksong, bufferL, bufferR, BUFFERSIZE);
	pos = m_currentBuffer * BUFFERSIZE * 2;
	for (i = 0; i < BUFFERSIZE; i++) {
		int samplel = bufferL[i];
		int sampler = bufferR[i];

		samplel = samplel < -32767 ? -32767 : samplel > 32767 ? 32767 : samplel;
		sampler = sampler < -32767 ? -32767 : sampler > 32767 ? 32767 : sampler;

		soundBuffer[pos++] = samplel;
		soundBuffer[pos++] = sampler;
	}

	// Prepare the buffer to be sent to the WaveOut API
	m_waveHeader[m_currentBuffer].lpData = (LPSTR) (&soundBuffer[2 * BUFFERSIZE * m_currentBuffer]);
	m_waveHeader[m_currentBuffer].dwBufferLength = BUFFERSIZE*2*2;
	waveOutPrepareHeader(m_hWaveOut,&m_waveHeader[m_currentBuffer],sizeof(WAVEHDR));

	// Send the buffer the the WaveOut queue
	waveOutWrite(m_hWaveOut,&m_waveHeader[m_currentBuffer],sizeof(WAVEHDR));

	m_currentBuffer++;
	if (m_currentBuffer >= REPLAY_NBSOUNDBUFFER) m_currentBuffer = 0;
}

static void CALLBACK waveOutProc(HWAVEOUT hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
		if (uMsg == WOM_DONE)
		{
			fillNextBuffer();
		}
}

int deviceOpen() {
	int i;
	WAVEFORMATEX pcmWaveFormat;

	short chn = 2;
	int rate = 44100;
	short bitpersample = 16;

	pcmWaveFormat.wFormatTag = WAVE_FORMAT_PCM; 
	pcmWaveFormat.nChannels = chn; 
	pcmWaveFormat.nSamplesPerSec = rate;
	pcmWaveFormat.nAvgBytesPerSec = chn * rate * (bitpersample / 8);
	pcmWaveFormat.nBlockAlign = 4; 
	pcmWaveFormat.wBitsPerSample = bitpersample; 
	pcmWaveFormat.cbSize = sizeof(WAVEFORMATEX);


	waveOutOpen(
		&m_hWaveOut,
		-1,
		&pcmWaveFormat,
		(DWORD_PTR)waveOutProc,
		0,
		CALLBACK_FUNCTION);

	m_currentBuffer = 0;
	for (i = 0; i < REPLAY_NBSOUNDBUFFER; i++) {
		fillNextBuffer();
	}

	return 0;
}
void deviceClose() {
	int i = 0;
	closing = TRUE;
	waveOutReset(m_hWaveOut);
	for (i = 0; i < REPLAY_NBSOUNDBUFFER; i++) {
		if ((m_waveHeader[i].dwFlags&WHDR_PREPARED) != 0)
			waveOutUnprepareHeader(m_hWaveOut, &m_waveHeader[i], sizeof(WAVEHDR));
	}

	waveOutClose(m_hWaveOut);
}

void deviceUpdate() {
	// winout handles itself...
}

