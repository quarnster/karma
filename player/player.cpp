// player.cpp : Defines the entry point for the console application.
//

#include <math.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "song.h"
// #include <winmm.h>
// #include "MidiPlayer.h"

#include "../karmalib/karma.h"

#define REPLAY_NBSOUNDBUFFER  3
#define BUFFERSIZE2  4096

int m_currentBuffer = 0;
WAVEHDR m_waveHeader[REPLAY_NBSOUNDBUFFER];
HWAVEOUT m_hWaveOut;
bool closing = false;
short soundBuffer[BUFFERSIZE2 * 2 * REPLAY_NBSOUNDBUFFER];
int tmp = 0;

#ifdef __GNUC__
static int bufferL[BUFFERSIZE2] __attribute__((aligned(32)));
static int bufferR[BUFFERSIZE2] __attribute__((aligned(32)));
#else
static __declspec(align(32)) int bufferL[BUFFERSIZE2];
static __declspec(align(32)) int bufferR[BUFFERSIZE2];
#endif

karma_Song *ksong = NULL;

// Internal WaveOut API callback function. We just call our sound handler ("playNextBuffer")
void	fillNextBuffer() {
	if (closing) return;
	// check if the buffer is already prepared (should not !)
	if (m_waveHeader[m_currentBuffer].dwFlags&WHDR_PREPARED)
		waveOutUnprepareHeader(m_hWaveOut,&m_waveHeader[m_currentBuffer],sizeof(WAVEHDR));

	memset(&bufferL, 0, BUFFERSIZE2 * sizeof(int));
	memset(&bufferR, 0, BUFFERSIZE2 * sizeof(int));

	karma_process(ksong, bufferL, bufferR, BUFFERSIZE2);
	int pos = m_currentBuffer * BUFFERSIZE2 * 2;
	for (int i = 0; i < BUFFERSIZE2; i++) {
		int samplel = bufferL[i];
		int sampler = bufferR[i];

		samplel = samplel < -32767 ? -32767 : samplel > 32767 ? 32767 : samplel;
		sampler = sampler < -32767 ? -32767 : sampler > 32767 ? 32767 : sampler;

		soundBuffer[pos++] = samplel;
		soundBuffer[pos++] = sampler;
	}

	// Prepare the buffer to be sent to the WaveOut API
	m_waveHeader[m_currentBuffer].lpData = (LPSTR) (&soundBuffer[2 * BUFFERSIZE2 * m_currentBuffer]);
	m_waveHeader[m_currentBuffer].dwBufferLength = BUFFERSIZE2*2*2;
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

void open() {
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


	int s = waveOutOpen(
		&m_hWaveOut,
		-1,
		&pcmWaveFormat,
		(DWORD_PTR)waveOutProc,
		0,					// User data.
		CALLBACK_FUNCTION);

	m_currentBuffer = 0;
	for (int i = 0; i < REPLAY_NBSOUNDBUFFER; i++) {
		fillNextBuffer();
	}

}
void close() {
	closing = true;
	waveOutReset(m_hWaveOut);					// Reset tout.
	for (int i=0;i<REPLAY_NBSOUNDBUFFER;i++) {
		if ((m_waveHeader[i].dwFlags&WHDR_PREPARED) != 0)
			waveOutUnprepareHeader(m_hWaveOut, &m_waveHeader[i], sizeof(WAVEHDR));
	}

	waveOutClose(m_hWaveOut);

}

int main(int argc, char **argv)
{
	if (argc != 1) {
		printf("usage: %s (that wasn't so hard now was it?)\n", argv[0]);
		return 0;
	}
	karma_init();
	ksong = karma_loadSong(&song[0]);

	ksong->samplesPlayed = 44100 * 0;
	while (ksong->currentEvent->next && ksong->currentEvent->next->time <= ksong->samplesPlayed) {
		ksong->currentEvent = ksong->currentEvent->next;
	}

	open();

	getch();

	close();

	karma_freeSong(ksong);
	karma_free();
	return 0;
}

