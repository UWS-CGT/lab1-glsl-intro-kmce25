#include <GL/glew.h>
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include "objLoader.h"

using namespace::std;

#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#define RT3D_VERTEX		0
#define RT3D_COLOUR		1
#define RT3D_NORMAL		2
#define RT3D_TEXCOORD   3
#define RT3D_INDEX		4

std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals; // Won't be used at the moment.

// Something went wrong - print error message and quit
void exitFatalError(const char *message)
{
    cout << message << " " << endl;
    cout << SDL_GetError();
    SDL_Quit();
    exit(1);
}


// Globals
// Real programs don't use globals :-D
// Data would normally be read from files
GLfloat vertices[] = {	-1.0f,0.0f,0.0f,
						0.0f,1.0f,0.0f,
						0.0f,0.0f,0.0f,
						0.0f,0.0f,0.0f,
						0.0f,-1.0f,0.0f,
						1.0f,0.0f,0.0f };
GLfloat colours[] = {	1.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 1.0f,
						1.0f, 0.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						1.0f, 0.0f, 0.0f };
GLfloat vertices2[] = {	0.0f,0.0f,0.0f,
						0.0f,-1.0f,0.0f,
						1.0f,0.0f,0.0f };
GLfloat vertices3[] = { 0.0f,0.0f,0.0f,
						0.0f,1.0f,0.0f,
						1.0f,0.0f,0.0f };
GLfloat vertices4[] = { -1.0f,0.0f,0.0f,
						0.0f,0.0f,0.0f,
						0.0f,-1.0f,0.0f };
GLfloat colours1[] = {	1.0f, 0.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						1.0f, 0.0f, 0.0f };
GLuint meshObjects[4];
GLuint VBOarray[6];


// loadFile - loads text file from file fname as a char* 
// Allocates memory - so remember to delete after use
// size of file returned in fSize
char* loadFile(const char *fname, GLint &fSize) {
	int size;
	char * memblock;

	// file read based on example in cplusplus.com tutorial
	// ios::ate opens file at the end
	ifstream file (fname, ios::in|ios::binary|ios::ate);
	if (file.is_open()) {
		size = (int) file.tellg(); // get location of file pointer i.e. file size
		fSize = (GLint) size;
		memblock = new char [size];
		file.seekg (0, ios::beg);
		file.read (memblock, size);
		file.close();
		cout << "file " << fname << " loaded" << endl;
	}
	else {
		cout << "Unable to open file " << fname << endl;
		fSize = 0;
		// should ideally set a flag or use exception handling
		// so that calling function can decide what to do now
		return nullptr;
	}
	return memblock;
}

// printShaderError
// Display (hopefully) useful error messages if shader fails to compile or link
void printShaderError(const GLint shader) {
	int maxLength = 0;
	int logLength = 0;
	GLchar *logMessage;

	// Find out how long the error message is
	if (!glIsShader(shader))
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
	else
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	if (maxLength > 0) { // If message has some contents
		logMessage = new GLchar[maxLength];
		if (!glIsShader(shader))
			glGetProgramInfoLog(shader, maxLength, &logLength, logMessage);
		else
			glGetShaderInfoLog(shader,maxLength, &logLength, logMessage);
		cout << "Shader Info Log:" << endl << logMessage << endl;
		delete [] logMessage;
	}
	// should additionally check for OpenGL errors here
}


GLuint initShaders(const char *vertFile, const char *fragFile) {
	GLuint p, f, v;

	char *vs,*fs;
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);	

	// load shaders & get length of each
	GLint vlen;
	GLint flen;
	vs = loadFile(vertFile,vlen);
	fs = loadFile(fragFile,flen);
	
	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv,&vlen);
	glShaderSource(f, 1, &ff,&flen);
	
	GLint compiled;

	glCompileShader(v);
	glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		cout << "Vertex shader not compiled." << endl;
		printShaderError(v);
	} 

	glCompileShader(f);
	glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		cout << "Fragment shader not compiled." << endl;
		printShaderError(f);
	} 
	
	p = glCreateProgram();
		
	glAttachShader(p,v);
	glAttachShader(p,f);

	glBindAttribLocation(p,RT3D_VERTEX,"in_Position");
	glBindAttribLocation(p,RT3D_COLOUR,"in_Color");
	glBindAttribLocation(p,RT3D_NORMAL,"in_Normal");
	glBindAttribLocation(p,RT3D_TEXCOORD,"in_TexCoord");

	glLinkProgram(p);
	glUseProgram(p);

	delete [] vs; // dont forget to free allocated memory
	delete [] fs; // we allocated this in the loadFile function...

	return p;
}


// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
        exitFatalError("Unable to initialize SDL"); 
	  
    // Request an OpenGL 3.3 context.
	// If you request a context not supported by your drivers, no OpenGL context will be created
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)
 
    // Create 800x600 window
    window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	if (!window) // Check window was created OK
        exitFatalError("Unable to create window");
 
    context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
    SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

void draw(SDL_Window * window) {
	// clear the screen - good to use grey as all white or all black triangles will still be visible
	glClearColor(0.5f,0.5f,0.5f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//glBindVertexArray(meshObjects[0]);	// Bind mesh VAO
	//glDrawArrays(GL_TRIANGLES, 0, 6);	// draw 3 vertices (one triangle)
	
	//glBindVertexArray(meshObjects[1]);	// Bind mesh VAO
	//glDrawArrays(GL_TRIANGLES, 0, 3);	// draw 3 vertices (one triangle)

	//glBindVertexArray(meshObjects[2]);	// Bind mesh VAO  --------------- 3rd triangle
	//glDrawArrays(GL_TRIANGLES, 0, 3);	// draw 3 vertices (one triangle)

	//glBindVertexArray(meshObjects[3]);	// Bind mesh VAO  --------------- 4th triangle
	//glDrawArrays(GL_TRIANGLES, 0, 3);	// draw 3 vertices (one triangle)

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    SDL_GL_SwapWindow(window); // swap buffers
}


void init(void) {
	// For this simple example we'll be using the most basic of shader programs
	initShaders("minimal.vert","minimal.frag");
		
	//meshObjects[0] = rt3d::createColourMesh(3, vertices, colours);

	// Going to create our mesh objects here
	//glGenVertexArrays(1, &meshObjects[0]);
	//glBindVertexArray(meshObjects[0]);
	//// generate and set up the VBO for the data
	//GLuint VBO;
	//glGenBuffers(1, &VBOarray[0]);
	//// VBO for vertex data
	//glBindBuffer(GL_ARRAY_BUFFER, VBOarray[0]);
	//glBufferData(GL_ARRAY_BUFFER, 18*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	//glVertexAttribPointer((GLuint)RT3D_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	//glEnableVertexAttribArray(RT3D_VERTEX);

	//// VBO for colour data
	//glGenBuffers(1, &VBOarray[1]);
	//glBindBuffer(GL_ARRAY_BUFFER, VBOarray[1]);
	//glBufferData(GL_ARRAY_BUFFER, 3*3*sizeof(GLfloat), colours, GL_STATIC_DRAW);
	//glVertexAttribPointer((GLuint)RT3D_COLOUR, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(RT3D_COLOUR);

	//glBindVertexArray(meshObjects[0]);
	////----------------------------------------------------------------------------------------------------------------
	
	//// Set up the second triangle
	//glGenVertexArrays(1, &meshObjects[1]);
	//glBindVertexArray(meshObjects[1]);	
	//// VBO for vertex data
	//glGenBuffers(1, &VBOarray[2]);
	//glBindBuffer(GL_ARRAY_BUFFER, VBOarray[2]);
	//glBufferData(GL_ARRAY_BUFFER, 3*3*sizeof(GLfloat), vertices2, GL_STATIC_DRAW);
	//glVertexAttribPointer((GLuint)RT3D_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	//glEnableVertexAttribArray(RT3D_VERTEX);
	//glBindVertexArray(meshObjects[0]);

	//----------------------------------------------------------------------------------------------------------------
	// 3rd Triangle
	//glGenVertexArrays(1, &meshObjects[2]);
	//glBindVertexArray(meshObjects[2]);

	//glGenBuffers(1, &VBOarray[3]);
	//glBindBuffer(GL_ARRAY_BUFFER, VBOarray[3]);
	//glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(GLfloat), vertices3, GL_STATIC_DRAW);
	//glVertexAttribPointer((GLuint)RT3D_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(RT3D_VERTEX);
	////glBindVertexArray(meshObjects[0]);

	//glGenBuffers(1, &VBOarray[4]);
	//glBindBuffer(GL_ARRAY_BUFFER, VBOarray[4]);
	//glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(GLfloat), colours1, GL_STATIC_DRAW);
	//glVertexAttribPointer((GLuint)RT3D_COLOUR, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(RT3D_COLOUR);

	//glBindVertexArray(meshObjects[0]);

	////----------------------------------------------------------------------------------------------------------------
	//glGenVertexArrays(1, &meshObjects[3]);
	//glBindVertexArray(meshObjects[3]);

	//glGenBuffers(1, &VBOarray[5]);
	//glBindBuffer(GL_ARRAY_BUFFER, VBOarray[5]);
	//glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(GLfloat), vertices4, GL_STATIC_DRAW);
	//glVertexAttribPointer((GLuint)RT3D_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(RT3D_VERTEX);
	////glBindVertexArray(meshObjects[0]);

	//glGenBuffers(1, &VBOarray[4]);
	//glBindBuffer(GL_ARRAY_BUFFER, VBOarray[4]);
	//glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(GLfloat), colours1, GL_STATIC_DRAW);
	//glVertexAttribPointer((GLuint)RT3D_COLOUR, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(RT3D_COLOUR);

	//glBindVertexArray(meshObjects[0]);



	
	bool res = loadOBJ("cube.obj", vertices, uvs, normals);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	
}


bool handleSDLEvent(SDL_Event const &sdlEvent)
{
	if (sdlEvent.type == SDL_QUIT)
		return false;
	if (sdlEvent.type == SDL_KEYDOWN)
	{
		// Can extend this to handle a wider range of keys
		switch( sdlEvent.key.keysym.sym )
		{
			case SDLK_ESCAPE:
				return false;
			default:
				break;
		}
	}
	return true;
}


// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[])
{
    SDL_Window * hWindow; // window handle
    SDL_GLContext glContext; // OpenGL context handle
    hWindow = setupRC(glContext); // Create window and render context 

	// Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit (1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

    
	SDL_Event sdlEvent;	// variable to detect SDL events
	bool running = true;
	while (running)		// the event loop
	{
		SDL_PollEvent(&sdlEvent);
		running = handleSDLEvent(sdlEvent);
		//update();			// not used yet!
		draw(hWindow);

	}


    SDL_DestroyWindow(hWindow);
    SDL_Quit();
    return 0;
}
