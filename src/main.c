#define PROJECT_PATH "test.sb2"

#include <stdio.h>
#include <stdlib.h>
#include <cmph.h>
#include "ut/dynarray.h"
#include "ut/uthash.h"

#include "types/primitives.h"
#include "types/value.h"
#include "types/block.h"

#include "zip_loader.h"
#include "project_loader.h"

#include "runtime.h"

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
