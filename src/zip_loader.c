/**
	Zip Loader
	  zip_loader.c

	This module reads SB2s (actually ZIP archives) produced by the Scratch editor, and it
	returns an array of "resources" and the project.json. A resource is represented by a
	struct Resource and can either be a handle a texture on the GPU produced from an image,
	or a (TODO: SVGs and WAVs). They are returned in an array, because the project.json
	references the resources with indices, making it easy for the parser to organize and
	load the resources into the peripherals.

	All of the actual decompression and parsing of the zip archives is done by minizip and
	zlib. This module just pulls out the files and processes them one at a time.
**/

#include <SDL2/SDL_opengl.h>
#include <SOIL2.h>

#define NOUNCRYPT
/* I just include the C files rather than the headers because this is the only module that
	 will be using minizip, and this file would be a really small module by itself */
#include "minizip/ioapi.c"
#include "minizip/unzip.c"
/* Even though I never use the header files for minizip, I've left them in the repo as
	 API reference. */

#include "types/primitives.h"

#include "zip_loader.h"

/*
	Returns an array of struct Resources, or NULL if it encountered a fatal error.
*/
struct Resource *loadSB2(const char *const path, char **const json, size_t *const jsonLen) {
	unzFile zip = unzOpen(path);
	if(zip == NULL) {
		printf("[FATAL]Could not open \"%s\" as a zip(sb2).\n", path);
		return NULL;
	}

	unz_global_info zipInfo;
	if(unzGetGlobalInfo(zip, &zipInfo) != UNZ_OK) {
		printf("[FATAL]Could not get the global info of \"%s\".\n", path);
		return NULL;
	}
	struct Resource *res = malloc((zipInfo.number_entry-1)*sizeof(struct Resource)); // don't check for only 1 entry, because there has to be at least 2 for every valid Scratch project
	if(res == NULL) {
		puts("[FATAL]Out of memory: could not allocate struct Resources.");
		return NULL;
	}

	int unzReturn;
	if(unzGoToFirstFile(zip) != UNZ_OK) {
		printf("[FATAL]Could not go to the first file in \"%s\".\n", path);
		return NULL;
	}
	do {
		unz_file_info fi;
		static char fileName[16] = {'\0'}; // "project.json" is 12 chars, and no asset will ever have a name longer than 15 chars
		if(unzGetCurrentFileInfo(zip, &fi, fileName, sizeof(fileName), NULL, 0, NULL, 0) != UNZ_OK) {
			printf("[FATAL]Something went wront when getting the info for a file in the SB2. The last file accessed (if any) was \"%s\".\n", fileName);
			return NULL;
		}

		uint32 i; bool isProjectJson;
		if(fileName[0] == 'p')
			isProjectJson = true;
		else {
			isProjectJson = false;
			char *ext;
			i = strtoul(fileName, &ext, 10);
			++ext;

			if(strncmp("png", ext, 3) == 0) res[i].format = BITMAP;
			else if(strncmp("jpg", ext, 3) == 0) res[i].format = BITMAP;
			else { printf("[FATAL]Resource \"%s\" is in an unrecognized format.\n", fileName); return NULL; }
		}

		char *file = malloc(fi.uncompressed_size);
		if(file == NULL) {
			printf("[FATAL]Out of memory: could not allocate memory for \"%s\"\n", fileName);
			return NULL;
		}
		if(unzOpenCurrentFile(zip) != UNZ_OK) {
			printf("[FATAL]Could not open \"%s\" in the SB2.\n", fileName);
			return NULL;
		}
		if(unzReadCurrentFile(zip, file, fi.uncompressed_size) < 0) {
			printf("[FATAL]Something when wrong when reading/decompressing \"%s\" in the SB2.\n", fileName);
			return NULL;
		}

		if(isProjectJson) {
			*json = file;
			*jsonLen = fi.uncompressed_size / sizeof(char);
		}
		else {
			switch(res[i].format) {
			case BITMAP:
				res[i].data.textureHandle = SOIL_load_OGL_texture_from_memory((unsigned char*)file, fi.uncompressed_size, 4, 0, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_INVERT_Y);
				if(res[i].data.textureHandle == 0) printf("[ERROR]Could not create texture for \"%s\".\n", fileName);
				break;
			case VECTOR: case SOUND: break;
			}
			free(file);
		}

	} while((unzReturn = unzGoToNextFile(zip)) == UNZ_OK);
	if(unzReturn != UNZ_END_OF_LIST_OF_FILE) {
		printf("[FATAL]Something went wrong when iterating through the files in \"%s\".\n", path);
		return NULL;
	}

	if(unzCloseCurrentFile(zip) != UNZ_OK)
		puts("[ERROR]Something went wrong when trying to close the project.json.");
	if(unzClose(zip) != UNZ_OK)
		puts("[ERROR]Something went wrong when trying to close the sb2(zip).");

	return res;
}
