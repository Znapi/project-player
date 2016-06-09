/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * phtg.c                                                              *
 *  Perfect HashTable Generator                                        *
 *                                                                     *
 * Uses cmph and specs.h in this directory to generate minimal perfect *
 * hashing functions for block op strings and block menus, and write   *
 * the resulting hashtables into header files.                         *
 *                                                                     *
 * NOTE: This module is not part of the final build, it generates      *
 *       parts of the final build.                                     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <string.h>
#include <cmph.h>

#include "specs.h"

#include "../types/primitives.h"
#include "../types/value.h"
#include "../types/block.h"
typedef blockhash hash;

#define TOTAL_OPS (sizeof(specs) / sizeof(struct BlockSpec))
static char *buffer[256];
static enum BlockType types[256];

int main(void) {
	// build array of op strings from spec table
	ufastest i;
	for(i = 0; i < TOTAL_OPS; ++i)
		buffer[i] = (char*)specs[i].opString;

	// use array of op strings to generate a minimal perfect hash function for them, and dump it to a file
	cmph_io_adapter_t *keySource = cmph_io_vector_adapter(buffer, TOTAL_OPS);
	FILE *mphfStream = fopen("blockops.mphf", "w");
	if(mphfStream == NULL) {
		puts("Could not open blockops.mphf!");
		return 1;
	}

	cmph_config_t *config = cmph_config_new(keySource);
	cmph_config_set_algo(config, CMPH_CHD);
	cmph_config_set_mphf_fd(config, mphfStream);

	cmph_t *mphf = cmph_new(config);
	cmph_config_destroy(config);
	cmph_dump(mphf, mphfStream);
	fclose(mphfStream);

	// make table of values for ops hash table, and write key hash mappings to file
	hash hash;
	FILE *mapStream = fopen("src/blockhash/map.txt", "w"); // file for writing key and hash pairs to
	if(mapStream == NULL) {
		puts("Could not open map.txt!");
		return 1;
	}

	fprintf(mapStream, "hash\tkey\n----\t---\n");

	for(i = 0; i < TOTAL_OPS; ++i) {
		hash = cmph_search(mphf, specs[i].opString, (cmph_uint32)strlen(specs[i].opString));
		buffer[hash] = (char*)specs[i].name;
		types[hash] = specs[i].type;
		fprintf(mapStream, "0x%x \t%s\n", hash, specs[i].opString);
	}
	fclose(mapStream);

	// write tables of values to C header files
	FILE *opstableStream = fopen("src/blockhash/opstable.c", "w");
	if(opstableStream == NULL) {
		puts("Could not open opstable.c!");
		return 1;
	}
	FILE *blocktypeStream = fopen("src/blockhash/typestable.c", "w");
	if(blocktypeStream == NULL) {
		puts("Could not open typestable.c!");
		return 1;
	}

	fprintf(opstableStream,
		"#pragma once\n"
		"// GENERATED FILE\n"
		"// see the README in this directory for details\n\n");
	fprintf(blocktypeStream,
		"#pragma once\n"
		"// GENERATED FILE\n"
		"// see the README in this directory for details\n\n"
		"enum BlockType {\n"
		"\tBLOCK_TYPE_S,\n"
		"\tBLOCK_TYPE_R,\n"
		"\tBLOCK_TYPE_B,\n"
		"\tBLOCK_TYPE_H,\n"
		"\tBLOCK_TYPE_F,\n"
		"\tBLOCK_TYPE_C, BLOCK_TYPE_CF,\n"
		"\tBLOCK_TYPE_E\n"
		"};\n\n");

	const uint16 totalHashes = cmph_size(mphf);
	fprintf(opstableStream, "const blockfunc opsTable[] = {\n");
	fprintf(blocktypeStream, "const enum BlockType blockTypesTable[] = {\n");
	for(i = 0; i < totalHashes; ++i) {
		fprintf(opstableStream, "\t%s,\n", buffer[i]);
		fprintf(blocktypeStream, "\tBLOCK_TYPE_");
		switch(types[i]) {
		case s:
			fprintf(blocktypeStream, "S");
			break;
		case r:
			fprintf(blocktypeStream, "R");
			break;
		case b:
			fprintf(blocktypeStream, "B");
			break;
		case h:
			fprintf(blocktypeStream, "H");
			break;
		case f:
			fprintf(blocktypeStream, "F");
			break;
		case c:
			fprintf(blocktypeStream, "C");
			break;
		case cf:
			fprintf(blocktypeStream, "CF");
			break;
		case e:
			fprintf(blocktypeStream, "E");
			break;
		}
		fprintf(blocktypeStream, ",\n");
	}
	fprintf(opstableStream, "};\n");
	fprintf(blocktypeStream, "};\n");
	fclose(opstableStream);
	fclose(blocktypeStream);

	cmph_destroy(mphf);

	return 0;
}
