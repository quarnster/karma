
#include <stdio.h>

struct MidiEvent {
	MidiEvent *next;
	long time;
	char data[3];
};
class MidiLoader {
	
public:
	MidiEvent *event;
	int eventNum;
	int timeformat;

	MidiLoader(FILE *in);
	~MidiLoader();

	void readTrack(FILE *fp);
	void addEvent(MidiEvent *e);
};