// player.cpp : Defines the entry point for the console application.
//

#include <math.h>
#include <windows.h>
#include <stdio.h>
// #include <winmm.h>
#include "MidiPlayer.h"

#include "../karmalib/waveform.h"
#include "../karmalib/Channel.h"

#define REPLAY_NBSOUNDBUFFER  3
#define BUFFERSIZE  4096

int m_currentBuffer = 0;
WAVEHDR m_waveHeader[REPLAY_NBSOUNDBUFFER];
HWAVEOUT m_hWaveOut;
bool closing = false;
short soundBuffer[BUFFERSIZE * 2 * REPLAY_NBSOUNDBUFFER];
int tmp = 0;

int samplesPlayed = 0;

#ifdef __GNUC__
static int bufferL[BUFFERSIZE] __attribute__((aligned(32)));
static int bufferR[BUFFERSIZE] __attribute__((aligned(32)));
#else
static __declspec(align(32)) int bufferL[BUFFERSIZE];
static __declspec(align(32)) int bufferR[BUFFERSIZE];
#endif

int phase = 0;
karma_Channel channel[16];
MidiEvent *currentEvent;
// Internal WaveOut API callback function. We just call our sound handler ("playNextBuffer")
void	fillNextBuffer() {
	if (closing) return;
	// check if the buffer is already prepared (should not !)
	if (m_waveHeader[m_currentBuffer].dwFlags&WHDR_PREPARED)
		waveOutUnprepareHeader(m_hWaveOut,&m_waveHeader[m_currentBuffer],sizeof(WAVEHDR));

	// Call the user function to fill the buffer with anything you want ! :-)
//	int *buf = (int*) (&soundBuffer[2 * BUFFERSIZE * m_currentBuffer]);

	while (currentEvent && (currentEvent->time - samplesPlayed) <= BUFFERSIZE) {
		int chn = currentEvent->data[0] & 0xf;
		int cmd = currentEvent->data[0] & 0xf0;

		if (cmd == 0xb0 && (currentEvent->data[1] == 120 || currentEvent->data[1] >= 123)) {
			for (int i = 0; i < 16; i++)
				karma_Channel_allNotesOff(&channel[i]);
		} else {
			karma_Event kevent;
			kevent.data[0] = currentEvent->data[0];
			kevent.data[1] = currentEvent->data[1];
			kevent.data[2] = currentEvent->data[2];
			kevent.deltaFrames = currentEvent->time - samplesPlayed;

			karma_Channel_addEvent(&channel[chn], &kevent);
		}
		currentEvent = currentEvent->next;
	}
	memset(&bufferL, 0, BUFFERSIZE * sizeof(int));
	memset(&bufferR, 0, BUFFERSIZE * sizeof(int));

	for (int i = 0; i < 16; i++) {
		karma_Channel_process(&channel[i], bufferL, bufferR, BUFFERSIZE);
	}

	int pos = m_currentBuffer * BUFFERSIZE * 2;
	for (int i = 0; i < BUFFERSIZE; i++) {
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
	samplesPlayed += BUFFERSIZE;
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

#define cMagic 		'CcnK'
#define fMagic		'FxCk'
#define bankMagic	'FxBk'

union fConvert {
	int ival;
	float fval;
};

extern int read4bytes(FILE *fp);
void readChunk(FILE *in, karma_Channel *chn) {
	int header = read4bytes(in);
	int length = read4bytes(in);

	char buffer[512];
	if (header !=  cMagic) {
		printf("ERROR!!! header != cMagic!!\n");
		printf("%d, %d\n", header, cMagic);
		return;
	}

	int header2 = read4bytes(in);
	switch (header2) {
		case bankMagic:
			{
				fseek(in, 3*4, SEEK_CUR);
				int programs = read4bytes(in);

				fseek(in, 128, SEEK_CUR);

				for (int i = 0; i < programs; i++) {
					printf("%d", &channel[i]);
					readChunk(in, &channel[i]);
				}
			}
			break;
		case fMagic:
			{
				fseek(in, 3*4, SEEK_CUR);
				int params = read4bytes(in);
				fseek(in, 28, SEEK_CUR);

				fConvert f;
				for (int i = 0; i < params; i++) {
					f.ival = read4bytes(in);
					karma_Program_setParameter(&chn->program, i, f.fval);
				}
			}
			break;
		default:
			printf("ERROR!!! Unknown chunk: %d\n", header);
			printf("len: %d\n", length);
			

			fseek(in, length - 4, SEEK_CUR);
			break;

	}
}
void readSettings(char *file) {
	FILE *in = NULL;
	if ((in = fopen(file, "rb")) == NULL) {
		printf("file %s could not be opened\n", file);
		return;
	}

	readChunk(in, NULL);
	fclose(in);
}
int main(int argc, char **argv)
{
/*
	karma_Channel c;
	karma_Channel_init(&c);
	karma_Waveform_initTables();

	open();
	getchar();
	close();
*/
	if (argc != 3) {
		printf("usage: %s file.mid file.fxb\n", argv[0]);
		return 0;
	}

	karma_Waveform_initTables();
	for (int i = 0; i < 16; i++) {
		printf("addr: %d\n", &channel[i]);
		karma_Channel_init(&channel[i]);
	}
	readSettings(argv[2]);

	MidiPlayer m(argv[1]);
	currentEvent = m.event;

	open();

	getchar();
	close();

	for (int i = 0; i < 16; i++) {
		karma_Channel_free(&channel[i]);
	}

	return 0;
}

