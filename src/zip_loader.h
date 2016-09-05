#pragma once

enum ResourceFormat {
	BITMAP, // PNG, JPG
	VECTOR, // SVG
	SOUND, // WAV
};

struct Resource {
	enum ResourceFormat format;
	union {
		GLuint textureHandle; // BITMAP
		// VECTOR
		// SOUND
	} data;
	union {
		struct {
			int width;
			int height;
		} dimensions;
	} metadata;
};

extern struct Resource *loadSB2(const char *const path, char **const json, size_t *const jsonLen);
