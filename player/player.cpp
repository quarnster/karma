// player.cpp : Defines the entry point for the console application.
//

#include <math.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "song.h"
// #include <winmm.h>
// #include "MidiPlayer.h"

#include "../karmalib/waveform.h"
#include "../karmalib/Channel.h"
#include "../karmalib/param.h"

#define REPLAY_NBSOUNDBUFFER  3
#define BUFFERSIZE2  4096

int m_currentBuffer = 0;
WAVEHDR m_waveHeader[REPLAY_NBSOUNDBUFFER];
HWAVEOUT m_hWaveOut;
bool closing = false;
short soundBuffer[BUFFERSIZE2 * 2 * REPLAY_NBSOUNDBUFFER];
int tmp = 0;

int samplesPlayed = 0;

#ifdef __GNUC__
static int bufferL[BUFFERSIZE2] __attribute__((aligned(32)));
static int bufferR[BUFFERSIZE2] __attribute__((aligned(32)));
#else
static __declspec(align(32)) int bufferL[BUFFERSIZE2];
static __declspec(align(32)) int bufferR[BUFFERSIZE2];
#endif

struct MidiEvent {
	MidiEvent *next;
	long time;
	unsigned char data[3];
};

karma_Channel channel[16];
MidiEvent *currentEvent;
MidiEvent *event;

// Internal WaveOut API callback function. We just call our sound handler ("playNextBuffer")
void	fillNextBuffer() {
	if (closing) return;
	// check if the buffer is already prepared (should not !)
	if (m_waveHeader[m_currentBuffer].dwFlags&WHDR_PREPARED)
		waveOutUnprepareHeader(m_hWaveOut,&m_waveHeader[m_currentBuffer],sizeof(WAVEHDR));


	while (currentEvent && (currentEvent->time - samplesPlayed) <= BUFFERSIZE2) {
		int chn = currentEvent->data[0] & 0xf;
		int cmd = currentEvent->data[0] & 0xf0;

		if (cmd == 0x90) {
			currentEvent->data[2] = 64;
		}
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
	memset(&bufferL, 0, BUFFERSIZE2 * sizeof(int));
	memset(&bufferR, 0, BUFFERSIZE2 * sizeof(int));

	for (int i = 0; i < 16; i++) {
		karma_Channel_process(&channel[i], bufferL, bufferR, BUFFERSIZE2);
	}

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
	samplesPlayed += BUFFERSIZE2;
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
void addEvent(MidiEvent *e) {
	if (!event || event->time > e->time) {
		// first element
		e->next = event;
		event = e;
	} else {
		MidiEvent *ev = event;
		while (ev->next && ev->next->time <= e->time) {
			ev = ev->next;
		}
		e->next = ev->next;
		ev->next = e;
	}
}
union fConvert {
	int ival;
	float fval;
};


void readData() {
	int pos = 0;
	// read instruments
	for (int i = 0; i < 16; i++) {
		fConvert f;
		for (int j = 0; j < 34; j++) {
			f.ival = song[pos] << 24 |  song[pos+1] << 16 | song[pos+2] << 8 | song[pos+3] << 0;
			pos += 4;
			karma_Program_setParameter(&channel[i].program, j, f.fval);
		}
	}

	// read midi-data
	int timeformat = song[pos] << 8 | song[pos+1] << 0; pos += 2;
	int eventNum = song[pos] << 8 | song[pos+1] << 0; pos += 2;

	float samplesPerTick = (60.0 / 120.0 * 44100.0) / (float) timeformat;

	while (eventNum) {
		long lastTime = 0;
		float fraction = 0;

		int num = song[pos] << 8 | song[pos+1] << 0; pos += 2;
		MidiEvent *root = NULL;
		MidiEvent *e = root;

		for (int i = 0; i < num; i++) {
			
			MidiEvent *ne = (MidiEvent*) malloc(sizeof(MidiEvent));
			ne->time = 0;
			int tmp = 0;
			do {
				tmp = song[pos]; pos++;
				ne->time = (ne->time << 7) | (tmp & 0x7F);
			} while (tmp & 0x80);

			if (ne->time > 0) {
				float samples = ne->time * samplesPerTick + fraction;
				ne->time = (long) samples;
				fraction = samples - ne->time;
			}

			ne->time += lastTime;

			lastTime = ne->time;

			if (!root) {
				e = root = ne;
			}
			e->next = ne;
			e = e->next;
		}

		int var = 1;
		root->data[0] = song[pos]; pos++;
		if ((root->data[0] & 0xf0) == 0xb0) {
			root->data[1] = song[pos]&0x7f; pos++;
			var++;
		}
		char last = 0;

		e = root;
		for (int i = 0; i < num; i++) {
			e->data[0] = root->data[0];
			e->data[1] = root->data[1];

			char curr = song[pos] + last; pos++;
			e->data[var] = curr&0x7f;
			last = e->data[var];

			MidiEvent *tmp = e;
			e = e->next;
			addEvent(tmp);
		}
		eventNum -= num;
	}
}

int main(int argc, char **argv)
{
	if (argc != 1) {
		printf("usage: %s (that wasn't so hard now was it?)\n", argv[0]);
		return 0;
	}

	karma_Waveform_initTables();
	int i = 0;

	for (i = 0; i < 16; i++) {
		karma_Channel_init(&channel[i]);
	}
	readData();

	currentEvent = event;


	open();

	getch();

	close();


	for (i = 0; i < 16; i++) {
		karma_Channel_free(&channel[i]);
	}

	while (event) {
		MidiEvent *e = event;
		event = event->next;
		free(e);
	}

	return 0;
}

