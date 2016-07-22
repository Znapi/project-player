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
#include "types/value.h"
#include "types/block.h"

#include "project_loader.h"

#include "runtime.h"
#include "peripherals.h"

int main(void) {
	initPeripherals(); // load the peripherals, creating a window, first to give the user immediate feedback that the app is starting, and the OGL context needs to exist for loading costumes
	if(loadProject(PROJECT_PATH)) return EXIT_FAILURE;

	initializeAskPrompt();

	puts("starting");
	restartGreenFlagThreads();
	puts("running");

	do {
		if(doRedraw && windowIsShowing)
			peripheralsOutputTick();
	} while(peripheralsInputTick() && stepThreads());
	puts("done.");

	// TODO: cleanup afterward
	destroyPeripherals();
	return EXIT_SUCCESS;
}
