#pragma once

#include "glm/glm.hpp"
#include "Ume/Renderer/VertexArray.h"

namespace Ume
{

	class Mesh
	{
	public:
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec3 Binormal;
			glm::vec2 Texcoord;
		};

		static_assert(sizeof(Vertex) == sizeof(float) * 14);
		static const int NumAttributes = 5;

		struct Index
		{
			uint32_t V1, V2, V3;
		};
		static_assert(sizeof(Index) == sizeof(uint32_t) * 3);

		Mesh(const std::string& path);
		~Mesh();

		inline const Ref<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		inline const std::string& GetFilePath() const { return m_FilePath; }
	private:
		std::vector<Vertex> m_Vertices;
		std::vector<Index> m_Indices;

		Ref<VertexArray> m_VertexArray;

		std::string m_FilePath;
	};
}

