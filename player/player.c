#include <math.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(a) usleep(a*1000)
#endif

#include "device.h"
#include "song.h"

#include <karma.h>

karma_Song *ksong = NULL;

int main(int argc, char **argv)
{
	karma_init();
	ksong = karma_loadSong(&song[0]);

	if (deviceOpen()) {
		fprintf(stderr, "Sorry, a device couldn't be opened... Exiting\n");
		karma_freeSong(ksong);
		karma_free();

		return 0;
	}

	while (ksong->currentEvent) {
		deviceUpdate();
		Sleep(10);
	}

	deviceClose();

	karma_freeSong(ksong);
	karma_free();

	return 0;
}

