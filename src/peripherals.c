/**
	Peripherals

	This module handles the graphics, sound, and user input.
**/

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SOIL.h>

#include "ut/uthash.h"

#include "types/primitives.h"
#include "types/costume.h"

static SDL_Window *wnd;
static Uint32 wndID;
static uint16 wndWidth, wndHeight;
static bool hasFocus, hasMouse;
static bool shouldQuit;
bool windowIsShowing;

static SDL_GLContext gl;

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

bool initPeripherals(void) {
	// Init SDL
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		printf("[FATAL]SDL could not initialize. SDL: %s\n", SDL_GetError());
		return true;
	}
	SDL_version v;
	SDL_GetVersion(&v);
	printf("[INFO]SDL version %d.%d.%d\n", v.major, v.minor, v.patch);

	// Init window
	wndWidth = 480;
	wndHeight = 360;
	wnd = SDL_CreateWindow("Unofficial Scratch Project Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, wndWidth, wndHeight, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
	if(wnd == NULL) {
		printf("[FATAL]SDL could not create new window. SDL: %s\n", SDL_GetError());
		return true;
	}

	// Init OpenGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	
	gl = SDL_GL_CreateContext(wnd);
	if(gl == NULL) {
		printf("[FATAL]SDL could not create an OpenGL context. SDL: %s\n", SDL_GetError());
		return true;
	}

	if(SDL_GL_SetSwapInterval(-1) != 0) {
		printf("[NOTE]SDL could not enable late swap tearing. Trying vsync instead. SDL: %s\n", SDL_GetError());
		if(SDL_GL_SetSwapInterval(1) != 0)
			printf("[WARNING]SDL could not enable vsync: %s\n", SDL_GetError());
	}

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	SDL_GL_SwapWindow(wnd);

	// finish setting OpenGL options
	//glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	wndID = SDL_GetWindowID(wnd);
	hasFocus = true; hasMouse = true; windowIsShowing = true;
	shouldQuit = false;

	printf("[INFO]OpenGL: version %s ; glsl version %s ; vendor %s ; renderer %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION), glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	return false;
}

void destroyPeripherals(void) {
	SDL_DestroyWindow(wnd);
	wnd = NULL;
}

static inline void handleWindowEvent(const SDL_WindowEvent wndEvent) {
	switch(wndEvent.event) {
	case SDL_WINDOWEVENT_FOCUS_GAINED:
		hasFocus = true;
		break;
	case SDL_WINDOWEVENT_FOCUS_LOST:
		hasFocus = false;
		break;
	case SDL_WINDOWEVENT_ENTER:
		hasMouse = true;
		break;
	case SDL_WINDOWEVENT_LEAVE:
		hasMouse = false;
		break;

	case SDL_WINDOWEVENT_SIZE_CHANGED: //same as SDL_WINDOWEVENT_RESIZED on my system
	case SDL_WINDOWEVENT_RESIZED:
		wndWidth = wndEvent.data1;
		wndHeight = wndEvent.data2;
		break;
	case SDL_WINDOWEVENT_EXPOSED:
		break;
	
	case SDL_WINDOWEVENT_HIDDEN:
	case SDL_WINDOWEVENT_MINIMIZED:
		windowIsShowing = false;
		break;

	case SDL_WINDOWEVENT_SHOWN:
	case SDL_WINDOWEVENT_RESTORED:
		windowIsShowing = true;
		break;

	case SDL_WINDOWEVENT_CLOSE:
		shouldQuit = true;
		break;
	}
}

static void drawWindow(void) {
	glClear(GL_COLOR_BUFFER_BIT);
}

bool peripheralsInputTick(void) {
	static SDL_Event e;
	while(SDL_PollEvent(&e) != 0) {
		switch(e.type) {
		case SDL_WINDOWEVENT:
			handleWindowEvent(e.window);
			break;
		case SDL_KEYDOWN:
			if(hasFocus)
				;
			break;
		case SDL_MOUSEMOTION:
			if(hasMouse)
				;
			break;
		case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP:
			if(hasMouse)
				;
			break;
		case SDL_MOUSEWHEEL: // treat mouse wheel as up/down arrow keys
			if(hasMouse)
				;
			break;
		case SDL_QUIT:
			shouldQuit = true;
			break;
		}
	}
	return !shouldQuit;
}

void peripheralsOutputTick(void) {
	drawWindow();
	SDL_GL_SwapWindow(wnd);
}
