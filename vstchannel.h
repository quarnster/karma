#ifndef __INCLUDED_KARMA_VSTCHANNEL_H
#define __INCLUDED_KARMA_VSTCHANNEL_H

#include "Channel.h"
#include "vstkarma.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


class VstChannel : public Channel {
private:
	Program *oldProgram;

public:
	VstChannel() : Channel() {
		oldProgram = NULL;
	}

	void delProgram(VstProgram *program) {
		active = false;

		if (this->program != NULL && strcmp(program->name, ((VstProgram*) this->program)->name) == 0) {
			this->program = oldProgram;
		}
	}

	void setProgram(VstProgram *program) {
		active = false;
		if (this->program != NULL && strcmp(program->name, ((VstProgram*) this->program)->name) != 0) {
			oldProgram = this->program;
			this->program = program;
		} else {
			this->program = oldProgram = program;
		}
	}
};

#endif