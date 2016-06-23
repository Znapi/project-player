#define JSON_PATH "project.json"

#include <stdlib.h>
#include <stdio.h>
#include <cmph.h>
#include "ut/dynarray.h"
#include "ut/uthash.h"

#include "types/primitives.h"
#include "types/value.h"
#include "types/block.h"

#include "project_loader.h"

#include "runtime.h"

static const char* loadFile(const char *const path, size_t *const length) {
	FILE *stream = fopen(path, "rb");
	if(stream == NULL) {
		printf("ERROR\n\tThe file `%s` could not be opened\n", path);
		fclose(stream);
		return 0;
	}
	fseek(stream, 0, SEEK_END);
	*length = (size_t)ftell(stream);
	char *chars = malloc(*length);
	rewind(stream);
	fread(chars, sizeof(char), *length, stream);
	fclose(stream);
	return chars;
}

void** load(ufastest *const nData) {
	size_t jsonLength;
	const char *const json = loadFile(JSON_PATH, &jsonLength);
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
	void **data = load(&nData);
	if(data == NULL) {
		puts("parsing error");
		return 1;
	}

	puts("starting");
	restartGreenFlagThreads();
	puts("running");
	while(stepThreads());
	puts("done.");

	while(nData != 0)
		free(data[--nData]);
	free(data);
}
