#pragma once

#include "MShader.h"
#include "Texture.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Material.h"
#include "ObjectBase.h"

#include <vector>
#include <string>

namespace moon {
	struct Vertex {
		Vector3 Position;
		Vector3 Normal;
		Vector2 TexCoords;
		Vector3 Tangent;
		Vector3 Bitangent;
	};

	class Mesh : public ObjectBase {
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		Material* material;
		unsigned int VAO;

		Mesh() {}
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) : ObjectBase("Mesh", MOON_AUTOID) {
			this->vertices = vertices;
			this->indices = indices;

			// set the vertex buffers and its attribute pointers
			setupMesh();
		}
		Mesh(const std::string &name, std::vector<Vertex> vertices, std::vector<unsigned int> indices) : ObjectBase(name, MOON_AUTOID) {
			this->vertices = vertices;
			this->indices = indices;

			// set the vertex buffers and its attribute pointers
			setupMesh();
		}
		~Mesh() { delete material; }

		void Draw(Shader* shader) {
			// bind appropriate textures
			unsigned int diffuseNr = 1;
			unsigned int specularNr = 1;
			unsigned int normalNr = 1;
			unsigned int heightNr = 1;
			unsigned int i;

			// binding textures
			//for (i = 0; i < textures.size(); i++) {
			//	// active proper texture unit before binding
			//	glActiveTexture(GL_TEXTURE0 + i);
			//	// retrieve texture number (the N in diffuse_textureN)
			//	std::string number;
			//	if (textures[i].type == TexType::diffuse)
			//		number = std::to_string(diffuseNr++);
			//	else if (textures[i].type == TexType::specular)
			//		number = std::to_string(specularNr++);
			//	else if (textures[i].type == TexType::normal)
			//		number = std::to_string(normalNr++);
			//	else if (textures[i].type == TexType::height)
			//		number = std::to_string(heightNr++);

			//	// now set the sampler to the correct texture unit
			//	glUniform1i(glGetUniformLocation(shader->ID, (name + number).c_str()), i);
			//	// and finally bind the texture
			//	glBindTexture(GL_TEXTURE_2D, textures[i].ID);
			//}

			// draw mesh
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			// always good practice to set everything back to defaults once configured.
			glActiveTexture(GL_TEXTURE0);
		}

	private:
		unsigned int VBO, EBO;

		// initializes all the buffer objects/arrays
		void setupMesh() {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			// load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// A great thing about structs is that their memory layout is sequential for all its items.
			// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a Vector3/2 array which
			// again translates to 3/2 floats which translates to a byte array.
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			// set the vertex attribute pointers
			// vertex Positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			// vertex normals
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			// vertex texture coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
			// vertex tangent
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
			// vertex bitangent
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

			glBindVertexArray(0);
		}
	};
}