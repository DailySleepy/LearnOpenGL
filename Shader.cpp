#include <string>
#include <fstream>
#include <sstream>

#include "GLCall.h"
#include "Shader.h"

string getFileName(const string& filePath)
{
	int lastSlashPos = filePath.find_last_of("/\\");
	int startPos = 0;

	if (string::npos != lastSlashPos)
		startPos = lastSlashPos + 1;

	string fileNameWithExtension = filePath.substr(startPos);

	int lastDotPos = fileNameWithExtension.find_last_of('.');

	if (string::npos == lastDotPos)
		return fileNameWithExtension;
	else
		return fileNameWithExtension.substr(0, lastDotPos);
}
string getFileExtension(const string& filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == string::npos) return "";
	if (dotPos == filePath.length() - 1) return "";

	return filePath.substr(dotPos);
}

Shader::Shader(const string& filePath) : m_ShaderID(0)
{
	if (getFileExtension(filePath) != ".shader")
	{
		cerr << "Wrong Shader construct function is used. Expected '.shader' extension for file: " << endl << '\t' << filePath << endl;
		throw runtime_error("Shader construction error: Incorrect file extension.");
	}

	shaderName = getFileName(filePath);
	ShaderSourceString shaderSource = parseMultiShader(filePath);
	if (shaderSource.geometrySource != "")
		m_ShaderID = createShader(shaderSource.vertexSource, shaderSource.geometrySource, shaderSource.fragmentSource);
	else
		m_ShaderID = createShader(shaderSource.vertexSource, shaderSource.fragmentSource);
}

Shader::Shader(const string& filePath, ShaderType type) : m_ShaderID(0)
{
	shaderName = getFileName(filePath);
	switch (type)
	{
		case ShaderType::Compute:
			m_ShaderID = createShader(parseShader(filePath));
			break;
		case ShaderType::Unknown:
			cout << "Cannot compile shader of Unknown type: " + filePath << endl;;
			break;
		default:
			cout << "Unhandled ShaderType provided: " + to_string(static_cast<int>(type)) + " for file: " + filePath << endl;
			break;
	}
}

Shader::Shader(const string& filePath1, const string& filePath2) : m_ShaderID(0)
{
	shaderName = getFileName(filePath1);
	m_ShaderID = createShader(parseShader(filePath1), parseShader(filePath2));
}

Shader::Shader(const string& filePath1, const string& filePath2, const string& filePath3) : m_ShaderID(0)
{
	shaderName = getFileName(filePath1);
	m_ShaderID = createShader(parseShader(filePath1), parseShader(filePath2), parseShader(filePath3));
}

Shader::~Shader()
{
	GLCall(glDeleteProgram(m_ShaderID));
}

void Shader::bind() const
{
	GLCall(glUseProgram(m_ShaderID));
}

void Shader::unbind() const
{
	GLCall(glUseProgram(0));
}

void Shader::bindUBO(unsigned int ubo, unsigned int binding)
{
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo));
}

void Shader::bindUBO(unsigned int ubo, string blockName, unsigned int binding)
{
	unsigned int blockIndex = glGetUniformBlockIndex(m_ShaderID, blockName.c_str());
	if (blockIndex == GL_INVALID_INDEX)
	{
		cerr << "Error: Uniform block '" << blockName << "' not found in shader program." << endl;
	}
	GLCall(glUniformBlockBinding(m_ShaderID, blockIndex, binding));
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo));
}

void Shader::bindSSBO(unsigned int ssbo, unsigned int binding)
{
	GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo));
}

void Shader::bindSSBO(unsigned int ssbo, string blockName, unsigned int binding)
{
	unsigned int blockIndex = glGetProgramResourceIndex(m_ShaderID, GL_SHADER_STORAGE_BLOCK, blockName.c_str());
	if (blockIndex == GL_INVALID_INDEX)
	{
		cerr << "Error: Shader storage block '" << blockName << "' not found in shader program." << endl;
	}
	GLCall(glShaderStorageBlockBinding(m_ShaderID, blockIndex, binding));
	GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo));
}

void Shader::setInt1(const string& name, int v0)
{
	GLCall(glUniform1i(getUniformLocation(name), v0));
}

void Shader::setFloat1(const string& name, float v0)
{
	GLCall(glUniform1f(getUniformLocation(name), v0));
}

void Shader::setFloat2(const string& name, float v0, float v1)
{
	GLCall(glUniform2f(getUniformLocation(name), v0, v1));
}

void Shader::setFloat3(const string& name, float v0, float v1, float v2)
{
	GLCall(glUniform3f(getUniformLocation(name), v0, v1, v2));
}

void Shader::setVec2(const string& name, vec2 vec)
{
	setFloat2(name, vec[0], vec[1]);
}

void Shader::setVec3(const string& name, vec3 vec)
{
	setFloat3(name, vec[0], vec[1], vec[2]);
}

void Shader::setVec4(const string& name, vec4 vec)
{
	GLCall(glUniform4f(getUniformLocation(name), vec[0], vec[1], vec[2], vec[3]));
}

void Shader::setMat4(const string& name, mat4 mat)
{
	GLCall(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value_ptr(mat)));
}

void Shader::setHandle(const string& name, uint64_t handle)
{
	GLCall(glUniformHandleui64ARB(getUniformLocation(name), handle));
}

void Shader::set(const UniformMap& uniforms)
{
	for (const auto& [name, v] : uniforms)
		visit([&](const auto& v) { set(name, v); }, v);
}

int Shader::getUniformLocation(const string& name) const
{
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
		return m_UniformLocationCache[name];

	GLCall(int location = glGetUniformLocation(m_ShaderID, name.c_str()));
	m_UniformLocationCache[name] = location;
	if (location == -1) cout << shaderName << ": warning: uniform '" << name << "' not exist\n";
	return location;
}

string Shader::parseShader(const string& filePath)
{
	ifstream file(filePath);
	if (!file.is_open())
	{
		cerr << "Error: Could not open the file: " << filePath << endl;
		return "";
	}

	stringstream fileContent;
	fileContent << file.rdbuf();

	file.close();
	return fileContent.str();
}

ShaderSourceString Shader::parseMultiShader(const string& filePath)
{
	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, GEOMETRY = 1, FRAGMENT = 2
	};

	ifstream stream(filePath);
	if (!stream.is_open())
	{
		cerr << "Error: Could not open the file: " << filePath << endl;
	}
	string line;
	stringstream ss[3];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") != string::npos)
		{
			if (line.find("vertex") != string::npos) type = ShaderType::VERTEX;
			else if (line.find("geometry") != string::npos) type = ShaderType::GEOMETRY;
			else if (line.find("fragment") != string::npos) type = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)type] << line << endl;
		}
	}
	return { ss[0].str(), ss[1].str(), ss[2].str() };
}

unsigned int Shader::compileShader(unsigned int type, const string& source)
{
	unsigned int id = 0;
	GLCall(id = glCreateShader(type));

	const char* src = source.c_str();
	GLCall(glShaderSource(id, 1, &src, nullptr));
	GLCall(glCompileShader(id));

	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		vector<char> message(length);
		GLCall(glGetShaderInfoLog(id, length, nullptr, message.data()));

		string name;
		switch (type)
		{
			case GL_VERTEX_SHADER: name = "vertex"; break;
			case GL_FRAGMENT_SHADER: name = "fragment"; break;
			case GL_GEOMETRY_SHADER: name = "geometry"; break;
			case GL_COMPUTE_SHADER: name = "compute"; break;
		}
		cout << "Failed to compile " << name << "shader, " << "Shader Name: " << shaderName << endl;
		cout << message.data() << endl;
		GLCall(glDeleteShader(id));
		return 0;
	}
	return id;
}

void Shader::checkLinkState(unsigned int program)
{
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

	if (linkStatus == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			vector<char> infoLog(infoLogLength);
			glGetProgramInfoLog(program, infoLogLength, nullptr, infoLog.data());
			cerr << "Program link error in " << "'" << shaderName << "'" << ": " << infoLog.data() << endl;
		}
		else
		{
			cerr << "Program link failed, but no info log is available." << endl;
		}
	}
}

unsigned int Shader::createShader(const string& computeShader)
{
	unsigned int program = 0;
	GLCall(program = glCreateProgram());

	unsigned int cs = compileShader(GL_COMPUTE_SHADER, computeShader);

	GLCall(glAttachShader(program, cs));
	GLCall(glLinkProgram(program));
	checkLinkState(program);

	GLCall(glValidateProgram(program));

	GLCall(glDeleteShader(cs));

	return program;
}

unsigned int Shader::createShader(const string& vertexShader, const string& fragmentShader)
{
	unsigned int program = 0;
	GLCall(program = glCreateProgram());

	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	checkLinkState(program);

	GLCall(glValidateProgram(program));

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

	return program;
}

unsigned int Shader::createShader(const string& vertexShader, const string& geometryShader, const string& fragmentShader)
{
	unsigned int program = 0;
	GLCall(program = glCreateProgram());

	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int gs = compileShader(GL_GEOMETRY_SHADER, geometryShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, gs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	checkLinkState(program);

	GLCall(glValidateProgram(program));

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));
	GLCall(glDeleteShader(gs));

	return program;
}

