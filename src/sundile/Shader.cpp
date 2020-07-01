//--
//-- Shader.cpp
//--
#include "Shader.h"
namespace sundile {
	std::vector<Shader> ShaderRegistry;

	namespace ShaderSystem {
		//--
		//-- Error Checking
		//--
		void checkError(const char* type, unsigned int item) {
			int success;
			char infoLog[512];

			if (type == "vertex" || type == "fragment")
			{
				glGetShaderiv(item, GL_COMPILE_STATUS, &success);
				if (!success)
				{
					glGetShaderInfoLog(item, 512, NULL, infoLog);
					std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
				}
			}
			if (type == "program")
			{
				glGetProgramiv(item, GL_LINK_STATUS, &success);
				if (!success) {
					glGetProgramInfoLog(item, 512, NULL, infoLog);
					std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
				}
			}
		}

		//--
		//-- Create and Initialize
		//--
		Shader init(Shader& s, const char* vertexPath, const char* fragmentPath) {

			// 1. retrieve the vertex/fragment source code from filePath
			std::string vertexCode;
			std::string fragmentCode;
			std::ifstream vShaderFile(vertexPath);
			std::ifstream fShaderFile(fragmentPath);

			// ensure ifstream objects can throw exceptions:
			try
			{
				// open files
				if (!vShaderFile) {
					std::cout << "ERROR::SHADER::VERTEX::FILE_NOT_SUCCESSFULLY_READ\n";
				}
				if (!fShaderFile) {
					std::cout << "ERROR::SHADER::FRAGMENT::FILE_NOT_SUCCESSFULLY_READ\n";
				}
				std::stringstream vShaderStream, fShaderStream;
				// read file's buffer contents into streams
				vShaderStream << vShaderFile.rdbuf();
				fShaderStream << fShaderFile.rdbuf();
				// close file handlers
				vShaderFile.close();
				fShaderFile.close();
				// convert stream into string
				vertexCode = vShaderStream.str();
				fragmentCode = fShaderStream.str();
			}
			catch (std::ifstream::failure e)
			{
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			}
			const char* vShaderCode = vertexCode.c_str();
			const char* fShaderCode = fragmentCode.c_str();

			// 2. compile shaders
			GLuint vertex, fragment;
			int success;
			char infoLog[512];

			// vertex Shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);

			glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::VERTEX::"<< vertexPath <<"::COMPILATION FAILED WITH INFO LOG:\n" << infoLog << std::endl;
			}

			// similiar for Fragment Shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);

			// print compile errors if any
			glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::FRAGMENT::" << fragmentPath << "::COMPILATION FAILED WITH INFO LOG:\n" << infoLog << std::endl;
			};

			// shader Program
			s = glCreateProgram();
			glAttachShader(s, vertex);
			glAttachShader(s, fragment);
			glLinkProgram(s);
			glGetProgramiv(s, GL_LINK_STATUS, &success);

			// delete the shaders as they're linked into our program now and no longer necessery
			glDeleteShader(vertex);
			glDeleteShader(fragment);

			if (success) {
				ShaderRegistry.push_back(s);
				return s;
			}
			else {
				glGetProgramInfoLog(s, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::"<<s<<"=(" << vertexPath << "," << fragmentPath << ")::COMPILATION FAILED WITH INFO LOG:\n" << infoLog << std::endl;
				return 0;
			}

		}
		Shader init(const char* vertexPath, const char* fragmentPath) {
			Shader s;
			return init(s, vertexPath, fragmentPath);
		}

		//--
		//-- Use
		//--
		void use(Shader s) {
			glUseProgram(s);
		};

		//--
		//-- Utility Functions
		//--
		void setBool(Shader s, const std::string& name, bool value)
		{
			glUniform1i(glGetUniformLocation(s, name.c_str()), (int)value);
		}
		// ------------------------------------------------------------------------
		void setInt(Shader s, const std::string& name, int value)
		{
			glUniform1i(glGetUniformLocation(s, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void setFloat(Shader s, const std::string& name, float value)
		{
			glUniform1f(glGetUniformLocation(s, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void setVec2(Shader s, const std::string& name, const glm::vec2& value)
		{
			glUniform2fv(glGetUniformLocation(s, name.c_str()), 1, &value[0]);
		}
		void setVec2(Shader s, const std::string& name, float x, float y)
		{
			glUniform2f(glGetUniformLocation(s, name.c_str()), x, y);
		}
		// ------------------------------------------------------------------------
		void setVec3(Shader s, const std::string& name, const glm::vec3& value)
		{
			glUniform3fv(glGetUniformLocation(s, name.c_str()), 1, &value[0]);
		}
		void setVec3(Shader s, const std::string& name, float x, float y, float z)
		{
			glUniform3f(glGetUniformLocation(s, name.c_str()), x, y, z);
		}
		// ------------------------------------------------------------------------
		void setVec4(Shader s, const std::string& name, const glm::vec4& value)
		{
			glUniform4fv(glGetUniformLocation(s, name.c_str()), 1, &value[0]);
		}
		void setVec4(Shader s, const std::string& name, float x, float y, float z, float w)
		{
			glUniform4f(glGetUniformLocation(s, name.c_str()), x, y, z, w);
		}
		// ------------------------------------------------------------------------
		void setMat2(Shader s, const std::string& name, const glm::mat2& mat)
		{
			glUniformMatrix2fv(glGetUniformLocation(s, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
		// ------------------------------------------------------------------------
		void setMat3(Shader s, const std::string& name, const glm::mat3& mat)
		{
			glUniformMatrix3fv(glGetUniformLocation(s, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
		// ------------------------------------------------------------------------
		void setMat4(Shader s, const std::string& name, const glm::mat4& mat)
		{
			glUniformMatrix4fv(glGetUniformLocation(s, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
	}
}