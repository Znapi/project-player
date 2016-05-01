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

#define PROJECT_PATH "/Users/znapi/Programming/project-player/src/perfect_hashes/"
#include "specs.h"

#include "../../types/primitives.h"
#include "../../types/value.h"
#include "../../types/block.h"
typedef blockhash hash;

#define TOTAL_OPS (sizeof(specs) / sizeof(struct BlockSpec))
static char *buffer[256];
//static char buf[32];

int main(void) {
	// build array of op strings from spec table
	//ufastest i, j;
	ufastest i;
	for(i = 0; i < TOTAL_OPS; ++i)
		buffer[i] = (char*)specs[i].opString;

	// use array of op strings to generate a minimal perfect hash function for them, and dump it to a file
	cmph_io_adapter_t *keySource = cmph_io_vector_adapter(buffer, TOTAL_OPS);
	FILE *mphfStream = fopen(PROJECT_PATH"blocks.mphf", "w");

	cmph_config_t *config = cmph_config_new(keySource);
	cmph_config_set_algo(config, CMPH_CHD);
	cmph_config_set_mphf_fd(config, mphfStream);

	cmph_t *mphf = cmph_new(config);
	cmph_config_destroy(config);
	cmph_dump(mphf, mphfStream);
	fclose(mphfStream);

	// make table of values for ops hash table, and write key hash mappings to file
	hash hash;
	FILE *mapStream = fopen(PROJECT_PATH"map.txt", "w"); // file for writing key and hash pairs to
	//FILE *macrosStream = fopen(PROJECT_PATH"hat_blocks.h", "w");

	fprintf(mapStream, "hash\tkey\n----\t---\n");
	//fprintf(macrosStream, "// generated file\n");

	for(i = 0; i < TOTAL_OPS; ++i) {
		//strcpy(buf, specs[i].opString);

		//hash = cmph_search(mphf, buf, (cmph_uint32)strlen(buf));
		hash = cmph_search(mphf, specs[i].opString, (cmph_uint32)strlen(specs[i].opString));
		buffer[hash] = (char*)specs[i].name;

		/*if(strncmp(buf, "when" , 4) == 0) {
			j = 0;
			do {
				if(buf[j] == ':')
					buf[j] = '$';
			} while(buf[++j] != '\0');
			fprintf(macrosStream, "#define BHASH_%s 0x%x\n", buf, hash);
			}*/

		fprintf(mapStream, "0x%x \t%s\n", hash, specs[i].opString);
	}
	fclose(mapStream);
	//fclose(macrosStream);

	// write table of values to a C header file
	FILE *opstableStream = fopen(PROJECT_PATH"opstable.c", "w");

	fprintf(opstableStream,
		"#pragma once\n"
		"// GENERATED FILE\n"
		"// see the README in this directory for details\n");

	const uint16 totalHashes = cmph_size(mphf);
	fprintf(opstableStream, "\nconst blockfunc opsTable[] = {\n");
	for(i = 0; i < totalHashes; ++i) {
		fprintf(opstableStream, "\t%s,\n", buffer[i]);
	}
	fprintf(opstableStream, "};\n");
	fclose(opstableStream);

	cmph_destroy(mphf);

	return 0;
}
