#ifndef __SHADERMANAGER_
#define __SHADERMANAGER_

#include <SFML/Window.hpp>
#include <stdio.h>

#if _MSC_VER
#define snprintf _snprintf
#endif

class ShaderManager
{
public:
	
	static GLuint shaderFromFile(char const * const * vertShaderPath, char const * const * fragShaderPath, int const vertShaderCount, int const fragShaderCount)
	{
		return readAndBuildProgram(vertShaderPath, fragShaderPath, vertShaderCount, fragShaderCount);
	}
	
	static GLuint shaderFromString(char const * const * vertShaderSource, char const * const * fragShaderSource, int const vertShaderCount, int const fragShaderCount)
	{
		return createShaderProgram(vertShaderSource, fragShaderSource, vertShaderCount, fragShaderCount);
	}
	
private:
	static GLuint readAndBuildProgram(char const * const * vertShaderPath, char const * const * fragShaderPath, int const vertShaderCount, int const fragShaderCount)
	{
		#ifndef MAX_SHADERS
		#define MAX_SHADERS 10
		#endif
		
		char ** newVertPath = (char**) malloc(sizeof(char*) * vertShaderCount);
		char ** newFragPath = (char**) malloc(sizeof(char*) * fragShaderCount);
		
		#define MAX_PATH_SIZE 800
		for(int i=0; i<vertShaderCount; i++)
			newVertPath[i] = (char*) malloc(sizeof(char) * MAX_PATH_SIZE);
		for(int i=0; i<fragShaderCount; i++)
			newFragPath[i] = (char*) malloc(sizeof(char) * MAX_PATH_SIZE);
		
#ifdef USING_XCODE3
		//need to reach outside the app bundle to get the shaders
		for(int i=0; i<vertShaderCount; i++)
			snprintf(newVertPath[i], MAX_PATH_SIZE, "../../%s", vertShaderPath[i]);
		for(int i=0; i<fragShaderCount; i++)
			snprintf(newFragPath[i], MAX_PATH_SIZE, "../../%s", fragShaderPath[i]);
#else
		for(int i=0; i<vertShaderCount; i++)
			snprintf(newVertPath[i], MAX_PATH_SIZE, "%s", vertShaderPath[i]);
		for(int i=0; i<fragShaderCount; i++)
			snprintf(newFragPath[i], MAX_PATH_SIZE, "%s", fragShaderPath[i]);
#endif
		
		if(vertShaderCount > MAX_SHADERS || fragShaderCount > MAX_SHADERS)
		{
			printf("This function can only build shaders with %i files. Exiting.\n", MAX_SHADERS);
			exit(1);
		}
		
		char * vertSource[MAX_SHADERS];
		char * fragSource[MAX_SHADERS];
		
		//read shader source
		for(int i=0; i<vertShaderCount; i++)
			vertSource[i] = readFile( newVertPath[i] );
		for(int i=0; i<fragShaderCount; i++)
			fragSource[i] = readFile( newFragPath[i] );
		
		//create shaders
		GLuint prog = createShaderProgram(vertSource, fragSource, vertShaderCount, fragShaderCount);
		
		for(int i=0; i<vertShaderCount; i++)
			free( (void*) vertSource[i]);
		for(int i=0; i<fragShaderCount; i++)
			free( (void*) fragSource[i]);
		
		return prog;
	}
	
	static GLuint createShaderProgram(char const * const * vertSource, char const * const * fragSource, int const vertShaderCount, int const fragShaderCount)
	{
#ifndef MAX_SHADERS
#define MAX_SHADERS 10
#endif
		if(vertShaderCount > MAX_SHADERS || fragShaderCount > MAX_SHADERS)
		{
			printf("This function can only build shaders with %i files. Exiting.\n", MAX_SHADERS);
			exit(1);
		}
		
		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		
		glShaderSource(vertShader, vertShaderCount, (char const **)vertSource, NULL);
		glShaderSource(fragShader, fragShaderCount, (char const **)fragSource, NULL);
		
		glCompileShader(vertShader);
		printShaderInfoLog(vertShader, vertShaderCount);
		glCompileShader(fragShader);
		printShaderInfoLog(fragShader, fragShaderCount);
		
		GLuint prog = glCreateProgram();
		glAttachShader(prog, vertShader);
		glAttachShader(prog, fragShader);
		
		glLinkProgram(prog);
		printProgramInfoLog(prog, vertShaderCount, fragShaderCount);
		
		return prog;
	}
	
	static void printShaderInfoLog(GLuint obj, int const shaderCount)
	{
		int infologLength = 0;
		int charsWritten  = 0;
		char *infoLog;
		
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
		
		if (infologLength > 0)
		{
			printf("Shader info log:\n");

			infoLog = (char *)malloc(infologLength);
			glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
			printf("%s\n",infoLog);
			free(infoLog);
		}
		else
		{
		}
	}
	
	static void printProgramInfoLog(GLuint obj, int const vertShaderCount, int const fragShaderCount)
	{
		int infologLength = 0;
		int charsWritten  = 0;
		char *infoLog;
		
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
		
		if (infologLength > 0)
		{
			printf("Shader program info log:\n");

			infoLog = (char *)malloc(infologLength);
			glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
			printf("%s\n",infoLog);
			free(infoLog);
		}
	}
	
	static FILE* sfopen(const char* name, const char *mode)
	{
		FILE *file = NULL;
		file = fopen(name, mode);
		
		if(file == NULL)
		{
			printf("Unable to open file '%s' in mode '%s'\n", name, mode);
			exit(1);
		}
		
		return file;
	}
	
	static char* readFile(char const * fileName)
	{
		FILE *file;
		
		//open file
		file = sfopen(fileName, "rb");
		
		//get size
		fseek(file, 0, SEEK_END);
		int fileDataLength = ftell(file);
		rewind(file);
		
		//get memory for the data
		char * fileData = new char[fileDataLength+1];
		fread(fileData, 1, fileDataLength, file);
		
		fclose(file);
		fileData[fileDataLength] = '\0';
		
		return fileData;
	}

};

#endif
