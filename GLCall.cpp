#include "GLCall.h"

void clearError()
{
	while (glGetError());
}
bool checkError()
{
	while (GLenum error = glGetError())
	{
		cout << "OpenGL error: " << error << endl;
		return true;
	}
	return false;
}