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

void** load(ufastest *const nData) {
	size_t jsonLength;
	const char *const json = loadSB2(PROJECT_PATH, &jsonLength);
	if(jsonLength == 0)
		return NULL;

	puts("parsing");
	void **data = loadProject(json, jsonLength, nData);
	puts("done parsing.");
	free((void*)json);

	return data;
}

int main(void) {
	ufastest nData;
	void **data = load(&nData); // TODO: return array of pointers that need freeing at end of runtime
	if(data == NULL) {
		puts("parsing error");
		return 1;
	}

	initializeAskPrompt();

	puts("starting");
	restartGreenFlagThreads();
	puts("running");
	while(stepThreads());
	puts("done.");

	while(nData != 0)
		free(data[--nData]);
	free(data);

	return 0;
}
