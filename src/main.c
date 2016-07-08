#define PROJECT_PATH "test.sb2"

/**
	A Scratch project player written in C

	This file is contains the entry point. It starts the loading process and then commands
	the runtime.

	There are two major stages to this programs run time: loading the project and running
	the project. Other stages (that are unimplemented) are idling (project loaded but not
	running), clean up (application recieves a close request), and project selection.

	This program is also broken up into two major parts: the peripherals (graphics, audio,
	controls) and the execution. I call just the execution part the runtime, and it's
	implementation is centered around runtime.c. It is only used when running the project.
	The peripherals half is not implemented yet, and is used during all stages except for
	loading.
**/

#include <stdio.h>
#include <stdlib.h>
#include <cmph.h>
#include "ut/uthash.h"

#include "types/primitives.h"
#include "ut/dynarray.h"

#include "types/value.h"
#include "types/block.h"
#include "types/thread.h"
#include "types/sprite.h"

#include "zip_loader.h"
#include "project_loader.h"

#include "runtime.h"

/**
	Loading a Scratch project is broken up into three parts:

	  * reading the project.json and resources like costumes into memory
	  * parsing the project.json and organizing resources and what was parsed into sprites
	  * loading resources into the peripherals and scripts into the runtime

	load() invokes the first and second parts. The second and third part are both done by
	project_loader.c so that the data parsed doesn't have to pass through this module to get
	to where it needs to go in the runtime or peripherals.

	No resources should be left over from loading. I've even been thinking about shoving all
	of the modules for loading into a dynamicly loaded library, so that even the code
	doesn't stick around.
**/
void load() {
	puts("reading");
	size_t jsonLength;
	const char *const json = loadSB2(PROJECT_PATH, &jsonLength);
	if(jsonLength == 0)
		return;

	puts("parsing and loading");
	loadProject(json, jsonLength);
	puts("done parsing and loading.");
	free((void*)json);
}

int main(void) {
	load();

	initializeAskPrompt();

	puts("starting");
	restartGreenFlagThreads();
	puts("running");
	while(stepThreads());
	puts("done.");

	// TODO: cleanup afterward
	return 0;
}
