/*
 * An example showing how to use the Karma softsynth
 * 
 * $Id: unixdevice.c,v 1.2 2004/01/01 16:06:19 Fredrik Ehnbom Exp $
 * Author : Fredrik Ehnbom
 * 
 * All rights reserved. Reproduction, modification, use or disclosure
 * to third parties without express authority is forbidden.
 * Copyright © Outbreak, Sweden, 2003, 2004.
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <memory.h>
#include <sys/time.h>
#include <stdio.h>

#include "device.h"

int bufferL[BUFFERSIZE] __attribute__((aligned(32)));
int bufferR[BUFFERSIZE] __attribute__((aligned(32)));

static int fd_dsp;
int deviceOpen() {
	/* TODO: error-checking... */
	int value;

	int n_fragments = 16;
	int fragment_size = 12;

	fd_dsp = open("/dev/dsp", O_WRONLY, 0);

	value = AFMT_S16_NE;
	ioctl(fd_dsp, SNDCTL_DSP_SETFMT, &value);

	value = (n_fragments << 16) | fragment_size;
	ioctl(fd_dsp, SNDCTL_DSP_SETFRAGMENT, &value);

	value = 16;
	ioctl(fd_dsp, SNDCTL_DSP_SAMPLESIZE, &value);

	value = 1; /* stereo */
	ioctl(fd_dsp, SNDCTL_DSP_STEREO, &value);

	value = 44100;
	ioctl(fd_dsp, SNDCTL_DSP_SPEED, &value);

	return 0;
}

void deviceClose() {
	close(fd_dsp);
}


short mixbuffer[BUFFERSIZE*2];
void deviceUpdate() {

	int pos = 0;
	int len;
	int i;
	audio_buf_info info;

	/* get how many bytes we can render without blocking */
	ioctl(fd_dsp, SNDCTL_DSP_GETOSPACE, &info);
	len = info.bytes / 4;

	if (len < 128) return;
	if (len > BUFFERSIZE) len = BUFFERSIZE;

	/* clear left/right-buffers and render sound */
	memset(bufferL, 0, len * 4);
	memset(bufferR, 0, len * 4);
	karma_process(ksong, bufferL, bufferR, len);

	/* clip the samples and put them in the final mixing buffer */
	for (i = 0; i < len; i++) {
		int samplel = bufferL[i];
		int sampler = bufferR[i];

		samplel = samplel < -32767 ? -32767 : samplel > 32767 ? 32767 : samplel;
		sampler = sampler < -32767 ? -32767 : sampler > 32767 ? 32767 : sampler;

		mixbuffer[pos++] = samplel;
		mixbuffer[pos++] = sampler;
	}
	write(fd_dsp, mixbuffer, len * 4);
}

