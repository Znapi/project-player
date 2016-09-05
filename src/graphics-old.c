#include "types/primitives.h"

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <SOIL.h>

#include <assert.h>

SDL_Window *wnd;
Uint32 wndID;
uint wndWidth, wndHeight;

bool hasFocus, hasMouse;
bool doRepaints;

SDL_GLContext gl;
GLint spriteShader;
GLint vertexAttribLoc;
GLuint vbo, vao;
GLuint fxUBO;
GLuint costume;

/* I/O convenience methods */

static bool readFile(const char *const filePath, char *const output) {
	assert(filePath != NULL); assert(output != NULL);
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

/* shader convenience methods */

static const char NO_GL_ERROR[] = "NO_ERROR";

static const char* getGlErrorAsString(const GLenum error) {
	switch(error) {
	case GL_NO_ERROR:
		return NO_GL_ERROR;
	case GL_INVALID_ENUM:
		return "INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "INVALID_OPERATION";
		//case GL_INVALID_FRAMEBUFFER_OPERATION:
		//return "INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "OUT_OF_MEMORY";
	case GL_STACK_UNDERFLOW:
		return "STACK_UNDERFLOW";
	case GL_STACK_OVERFLOW:
		return "STACK_OVERFLOW";
	default:
		return "Unrecognized error";
	}
}

#define printGLerror(onError) {															\
		const char *const e = getGlErrorAsString(glGetError()); \
		if(e != NO_GL_ERROR) {																	\
			printf("[!!!FATAL!!!]GL ERROR: %s\n", e);							\
			onError;																							\
		}																												\
	}

static GLint compileShader(const char *const shaderPath, const GLenum type) {
	assert(shaderPath != NULL);
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
}

static bool newPlayer(/* const char *const projectPath */void) {
	wnd = SDL_CreateWindow("Unofficial Project Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 480, 360, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
	if(wnd == NULL) {
		printf("[FATAL]SDL could not create new window. SDL: %s\n", SDL_GetError());
		return true;
	}
	wndID = SDL_GetWindowID(wnd);
	wndWidth = 480;
	wndHeight = 360;
	doRepaints = hasFocus = hasMouse = true;

	// create OpenGL context
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
	//glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#define SHADER_DIR "src/"
	GLint vertexShader = compileShader(SHADER_DIR"sprite.vert.glsl", GL_VERTEX_SHADER);
	GLint fragmentShader = compileShader(SHADER_DIR"sprite.frag.glsl", GL_FRAGMENT_SHADER);
	if(vertexShader == 0 || fragmentShader == 0) // if shaders could not be compiled
		return true;
	spriteShader = createShaderProgram(vertexShader, fragmentShader);

	vertexAttribLoc = glGetAttribLocation(spriteShader, "A");
	if(vertexAttribLoc == -1) {
		puts("[FATAL]Vertex attribute is not active");
		return true;
	}

	GLfloat vertexData[] = {
		/*
		// pos            // tex coord
		-46.0f,  50.0f,   0.0f,   0.0f,
		-46.0f, -50.0f,   0.0f,   200.0f,
		46.0f, -50.0f,   184.0f, 200.0f,

		46.0f, -50.0f,   184.0f, 200.0f,
		46.0f,  50.0f,   184.0f, 0.0f,
		-46.0f,  50.0f,   0.0f,   0.0f
		*/

		// pos            // tex coord
		-46.0f,  50.0f,   0.0f,  0.0f,
		-46.0f, -50.0f,   0.0f,  1.0f,
		46.0f, -50.0f,   1.0f,  1.0f,

		46.0f, -50.0f,   1.0f,  1.0f,
		46.0f,  50.0f,   1.0f,  0.0f,
		-46.0f,  50.0f,   0.0f,  0.0f
	};
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo); /* TODO: error checking */

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexAttribLoc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)NULL);
	glEnableVertexAttribArray(vertexAttribLoc);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLfloat fxData[] = {
		0.0f, //color
		0.0f, //brightness
		0.0f, //ghost
		1.0f, //pixelate
		1.0f, //mosaic
		0.0f, //fisheye
		0.0f, //whirl
	};
	glGenBuffers(1, &fxUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, fxUBO);
	glBufferData(GL_UNIFORM_BUFFER, 4*7, fxData, GL_STATIC_DRAW);
	GLuint fxBlockIndex = glGetUniformBlockIndex(spriteShader, "FX");
	if(fxBlockIndex == GL_INVALID_INDEX) {
		puts("[FATAL]Graphic effects uniform block is not active");
		return true;
	}
	glUniformBlockBinding(spriteShader, 0, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, fxUBO);

	int width, height;
	ubyte* image = SOIL_load_image("costume.png", &width, &height, 0, SOIL_LOAD_RGBA);
	if(image == NULL)
		puts("[ERROR]Could not load costume");

	glGenTextures(1, &costume);
	glBindTexture(GL_TEXTURE_2D, costume);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(image);

	return false;
}

static void destroyPlayer(void) {
	SDL_DestroyWindow(wnd);
	wnd = NULL;
}

/* returns true if player was destroyed */
static bool playerHandleWindowEvent(const SDL_WindowEvent wndEvent) {
	if(wndEvent.windowID == wndID) {
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

			//case SDL_WINDOWEVENT_SIZE_CHANGED: //same as SDL_WINDOWEVENT_RESIZED on my system
		case SDL_WINDOWEVENT_RESIZED:
			wndWidth = wndEvent.data1;
			wndHeight = wndEvent.data2;
			break;
		case SDL_WINDOWEVENT_EXPOSED:
			break;

		case SDL_WINDOWEVENT_HIDDEN:
		case SDL_WINDOWEVENT_MINIMIZED:
			doRepaints = false;
			break;

		case SDL_WINDOWEVENT_SHOWN:
		case SDL_WINDOWEVENT_RESTORED:
			doRepaints = true;
			break;

		case SDL_WINDOWEVENT_CLOSE:
			destroyPlayer();
			return true;
			break;
		}
	}
	return false;
}

static void playerHandleKeyboardEvent(const SDL_KeyboardEvent keyEvent) {
	if(keyEvent.windowID == wndID) {
		switch(keyEvent.keysym.sym) {
		case SDLK_a:
			;
			break;
		}
	}
}

static void render(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(spriteShader);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//glUseProgram(0);
}

/* For adding the ability to run multiple instances of the player in the future */
//Player *instances; // dynamically sized array of player instances
//ubyte nInstances = 0;

/*void newInstance(const char *title, uint width, uint height) {
	instances = reallocf(instances, sizeof(Player)*(nInstances+1)); // allocate new array

	// create new window in last array slot
	instances[nInstances] = newPlayer(title, width, height);
	++nInstances;
	}

	void removeInstance(ubyte index) {
	memmove(instances+index, instances+index+1, sizeof(Player)*(nInstances-index-1));
	instances = reallocf(instances, sizeof(Player)*(nInstances-1));
	--nInstances;
	}*/

int main(void) {
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		printf("[FATAL]SDL could not initialize. SDL: %s\n", SDL_GetError());
		return 1;
	}
	{
		SDL_version v;
		SDL_GetVersion(&v);
		printf("[INFO]SDL version %d.%d.%d\n", v.major, v.minor, v.patch);
	}

	/*instances = malloc(sizeof(Player));
		instances[0].wnd = SDL_CreateWindow("Unofficial Project Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 480, 360, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
		if(instances[0].wnd == NULL) {
		printf("[FATAL ERROR]SDL could not create window: %s\n", SDL_GetError());
		return 1;
		}
		instances[0].wndID = SDL_GetWindowID(instances[0].wnd);
		instances[0].wndWidth = 640;
		instances[0].wndHeight = 480;
		instances[0].hasFocus = instances[0].hasMouse = true;
		nInstances = 1;*/

	if(newPlayer())
		return 1;
	printf("[INFO]OpenGL: version %s ; glsl version %s ; vendor %s ; renderer %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION), glGetString(GL_VENDOR), glGetString(GL_RENDERER));

	SDL_Event e;
	bool shouldQuit = false;

	while(!shouldQuit) {
		if(SDL_PollEvent(&e)) {
			switch(e.type) {
			case SDL_WINDOWEVENT:
				/*for(ubyte i = 0; i < nInstances; ++i) {
					if(playerHandleWindowEvent(instances+i, e.window))
					removeInstance(i);
					}*/
				playerHandleWindowEvent(e.window);
				break;
			case SDL_KEYDOWN:
				if(hasFocus) {
					/*for(ubyte i = 0; i < nInstances; ++i)
						playerHandleKeyboardEvent(instances+i, e.key);*/
					playerHandleKeyboardEvent(e.key);
				}
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
				if(wnd != NULL)
					destroyPlayer();
				break;
			}
		}
		render(); // rendering needs to be done even when doRepaints==false so that `touching` blocks work
		if(doRepaints)
			SDL_GL_SwapWindow(wnd); // don't update the window if doRepaints==false to save a little bit of processing
	}

	glDeleteProgram(spriteShader);
	SDL_Quit();
	return 0;
}
