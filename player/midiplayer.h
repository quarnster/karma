
struct MidiEvent {
	MidiEvent *next;
	long time;
	char data[3];
};
class MidiPlayer {
	
public:
	MidiEvent *event;

	MidiPlayer(char *file);
	~MidiPlayer();

	void readTrack(FILE *fp);
	void addEvent(MidiEvent *e);
};