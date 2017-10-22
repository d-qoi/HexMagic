#ifndef __GLHELPER
#define __GLHELPER

static void checkGLError(char const * ident = "")
{
	GLenum errCode;
	
	if ((errCode = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL Error (%s): ", ident);
		switch (errCode)
		{
		case GL_INVALID_ENUM:
			fprintf(stderr, "GL_INVALID_ENUM\n");
			break;
		case GL_INVALID_VALUE:
			fprintf(stderr, "GL_INVALID_VALUE\n");
			break;
		case GL_INVALID_OPERATION:
			fprintf(stderr, "GL_INVALID_OPERATION\n");
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			fprintf(stderr, "GL_INVALID_FRAMEBUFFER_OPERATION\n");
			break;
		case GL_OUT_OF_MEMORY:
			fprintf(stderr, "GL_OUT_OF_MEMORY\n");
			break;
		}
	}
}

#endif