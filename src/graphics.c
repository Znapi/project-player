#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "types/primitives.h"
#include "ut/uthash.h"
#include "ut/dynarray.h"

#include "types/thread.h"
#include "types/costume.h"
#include "types/sprite.h"

#include "graphics.h"

enum DrawableType {
	DRAWABLE_SPRITE,
	//VARIABLE_WATCHER,
	//LIST_WATCHER,
	//ASK_PROMPT
};

struct Drawable {
	enum DrawableType type;
	union {
		SpriteContext *sprite;
		//watcher
		//ask prompt
	} obj;
	struct Drawable *next;
};
typedef struct Drawable Drawable;

static Drawable *objects; // linked list of Drawables that should be drawn, with the object on the bottom layer at index 0

// TODO: move this procedure
/*static bool readFile(const char *const filePath, char *const output) {
	printf("[INFO]Loading file %s\n", filePath);
	long fileSize;
	FILE *file = fopen(filePath, "r");
	if(file == NULL) {
		fclose(file);
		return true;
	}
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	rewind(file);
	fread(output, sizeof(char), fileSize, file);
	fclose(file);

	//printf("Contents:\n%s\n\n", output);
	return false;
	}

static GLint compileShader(const char *const shaderPath, const GLenum type) {
	char shaderSrc[4096] = {0};
	GLchar infoLog[512] = {0};

	memset(shaderSrc, 0, sizeof(shaderSrc));
	memset(infoLog, 0, sizeof(infoLog));
	if(readFile(shaderPath, shaderSrc)) {
		printf("[FATAL]Could not read shader file `%s`\n", shaderPath);
		return 0;
	}
	//printf("Shader contents:\n%s\n\n", shaderSrc);

	GLint success;
	GLint shader = glCreateShader(type);
	if(shader == 0)
		return 0;

	const GLchar* shaderSource = &(((GLchar*)shaderSrc)[0]);

	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		printf("[FATAL]Shader(%s) compilation failed.", shaderPath);
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf(" OpenGL:\n``%s\b\b``\n", infoLog);
		return 0;
	}
	return shader;
}

static inline GLint createShaderProgram(const GLint vertex, const GLint fragment) {
	GLint shader = glCreateProgram();
	glAttachShader(shader, vertex);
	glAttachShader(shader, fragment);
	glLinkProgram(shader);
	return shader;
	}*/

static inline void drawSprite(SpriteContext *const sprite) {
	//glBindTexture(sprite->currentCostume->textureHandle);
	// if costume is vector
	//   if needs re-rasterization
	//     free outdated texture and render the svg to a new texture
	//glBindVertexArray(sprite->currentCostume->vaoHandle);
	// upload gfx parameters, etc., and run sprite shader

	// if sprite is "saying" or "thinking" something
	// upload text, vertex data, etc., and run say or think bubble shader
}

void initGraphics(void) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
