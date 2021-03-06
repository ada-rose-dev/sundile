#include "ShaderSystem.h"

#ifndef S_MESH
#define S_MESH

SYSTEM(MeshSystem)
	namespace {
		void checkError() {
			GLenum error = glGetError();
			if (error) {
				std::cout << "OPENGL ERROR::" << error << std::endl;
				SUNDILE_DEBUG_BREAK
			}
		}

		Mesh setupMesh(Mesh m)
		{
			auto& VAO = m.VAO;
			auto& VBO = m.VBO;
			auto& EBO = m.EBO;
			auto& vertices = m.vertices;
			auto& indices = m.indices;

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			// vertex positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			// vertex normals
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			// vertex texture coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

			glBindVertexArray(0);
			checkError();
			return m;
		}
	}


	Mesh create(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
		Mesh m(vertices, indices, textures);
		m = setupMesh(m);
		return m;
	}

	void Draw(Mesh&& m, Shader s)
	{
		auto textures = m.textures;
		auto indices = m.indices;
		auto vertices = m.vertices;

		//Set textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
			checkError();
			// retrieve texture number (the N in diffuse_textureN)
			std::string number;
			std::string name = textures[i].type;
			if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++);

			//e.g. "material.texture_diffuse1"
			//NOTE: Shaders must declare uniform sampler2Ds using this naming convention or this will not work!
			ShaderSystem::setFloat(s, (/*"material." +*/ name + number).c_str(), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
			checkError();
		}
		glActiveTexture(GL_TEXTURE0);
		checkError();

		// draw mesh
		glBindVertexArray(m.VAO);
		checkError();

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		checkError();
		glBindVertexArray(0);
	}
END_SYSTEM
#endif