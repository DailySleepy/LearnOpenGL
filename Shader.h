#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>
#include <variant>

using namespace std;
using namespace glm;

struct ShaderSourceString
{
	string vertexSource;
	string geometrySource;
	string fragmentSource;
};

enum class ShaderType
{
	Unknown = 0,
	Vertex,
	TessellationControl,
	TessellationEvaluation,
	Geometry,
	Fragment,
	Compute
};

class Shader
{
private:
	string shaderName;
	unsigned int m_ShaderID;
	mutable unordered_map<string, unsigned int> m_UniformLocationCache;
public:
	Shader(const string& filePath);
	Shader(const string& filePath, ShaderType type);
	Shader(const string& filePath1, const string& filePath2);
	Shader(const string& filePath1, const string& filePath2, const string& filePath3);
	~Shader();

	void bind() const;
	void unbind() const;
	void bindUBO(unsigned int ubo, unsigned int binding);
	void bindUBO(unsigned int ubo, string blockName, unsigned int binding);
	void bindSSBO(unsigned int ssbo, unsigned int binding);
	void bindSSBO(unsigned int ssbo, string blockName, unsigned int binding);
	void setInt1(const string& name, int v0);
	void setFloat1(const string& name, float v0);
	void setFloat2(const string& name, float v0, float v1);
	void setFloat3(const string& name, float v0, float v1, float v2);
	void setVec2(const string& name, vec2 vec);
	void setVec3(const string& name, vec3 vec);
	void setVec4(const string& name, vec4 vec);
	void setMat4(const string& name, mat4 mat);
	void setHandle(const string& name, uint64_t handle);

	template<typename T>
	void set(const string name, T v) = delete;
	void set(const string name, int v) { setInt1(name, v); }
	void set(const string name, bool v) { setInt1(name, v); }
	void set(const string name, float v) { setFloat1(name, v); }
	void set(const string name, double v) { setFloat1(name, v); }
	void set(const string name, uint64_t v) { setHandle(name, v); }
	void set(const string name, vec2 v) { setVec2(name, v); }
	void set(const string name, vec3 v) { setVec3(name, v); }
	void set(const string name, vec4 v) { setVec4(name, v); }
	void set(const string name, mat4 v) { setMat4(name, v); }
	void set(const string name, float v0, float v1) { setFloat2(name, v0, v1); }
	void set(const string name, float v0, float v1, float v2) { setFloat3(name, v0, v1, v2); }
	using UniformValue = variant<int, bool, float, double, uint64_t, vec2, vec3, vec4, mat4>;
	using UniformMap = unordered_map<string, UniformValue>;
	void set(const UniformMap& uniforms);

	unsigned int getShaderID() const { return m_ShaderID; }

private:
	int getUniformLocation(const string& name) const;
	string parseShader(const string& filePath);
	ShaderSourceString parseMultiShader(const string& filePath);
	unsigned int compileShader(unsigned int type, const string& source);
	void checkLinkState(unsigned int program);
	unsigned int createShader(const string& computeShader);
	unsigned int createShader(const string& vertexShader, const string& fragmentShader);
	unsigned int createShader(const string& vertexShader, const string& fragmentShader, const string& geometryShader);
};
