/**
	Zip Loader
	  zip_loader.c

	This module reads the .sb2 compressed zip archives produced by the Scratch editors and
	loads the files into memory, so that other modules related to loading projects can act
	on them.

	All of the actual decompression and parsing of the zip archives is done by minizip and
	zlib. This module simply uses those modules to get the files it wants out of the zip and
	sort them.
**/

#define NOUNCRYPT
#include "minizip/ioapi.c"
#include "minizip/unzip.c"
/* Even though I never use the header files for minizip, I've left them in the repo as
	 API reference. */

char* loadSB2(const char *const path, size_t *const len) {
	unzFile zip = unzOpen(path);
	if(zip == NULL) {
		printf("[FATAL]Could not open \"%s\" as a zip(sb2).\n", path);
		return NULL;
	}

	if(unzLocateFile(zip, "project.json", 1) != UNZ_OK) {
		printf("[FATAL]Could not locate project.json in \"%s\".\n", path);
		return NULL;
	}
	if(unzOpenCurrentFile(zip) != UNZ_OK) {
		puts("[FATAL]Could not open project.json in the given sb2.");
		return NULL;
	}
	unz_file_info fi;
	if(unzGetCurrentFileInfo(zip, &fi, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
		puts("[FATAL]Something went wront when getting the info for the project.json");
		return NULL;
	}
	*len = fi.uncompressed_size;
	char *const json = malloc(fi.uncompressed_size);
	if(unzReadCurrentFile(zip, json, fi.uncompressed_size) < 0) {
		puts("[FATAL]Something when wrong when reading/decompressing the project.json");
		return NULL;
	}

	if(unzCloseCurrentFile(zip) != UNZ_OK)
		puts("[ERROR]Something went wrong when trying to close the project.json.");
	if(unzClose(zip) != UNZ_OK)
		puts("[ERROR]Something went wrong when trying to close the sb2(zip).");

	return json;
}
