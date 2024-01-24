#include "umepch.h"
#include "Mesh.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include "Ume/Renderer/Buffer.h"

namespace Ume
{
    namespace {
        const unsigned int ImportFlags =
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_SortByPType |
            aiProcess_PreTransformVertices |
            aiProcess_GenNormals |
            aiProcess_GenUVCoords |
            aiProcess_OptimizeMeshes |
            aiProcess_Debone |
            aiProcess_ValidateDataStructure;
    }

    struct LogStream : public Assimp::LogStream
    {
        static void Initialize()
        {
            if (Assimp::DefaultLogger::isNullLogger())
            {
                Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
                Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
            }
        }

        void write(const char* message) override
        {
            UME_CORE_ERROR("Assimp error: {0}", message);
        }
    };

    Mesh::Mesh(const std::string& filename)
        : m_FilePath(filename)
    {
        LogStream::Initialize();

        UME_CORE_INFO("Loading mesh: {0}", filename);

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(filename, ImportFlags);
        if (!scene || !scene->HasMeshes())
            UME_CORE_ERROR("Failed to load mesh file: {0}", filename);

        aiMesh* mesh = scene->mMeshes[0];
        UME_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.");
        UME_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.");

        m_Vertices.reserve(mesh->mNumVertices);

        for (size_t i = 0; i < m_Vertices.capacity(); i++)
        {
            Vertex vertex;
            vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

            if (mesh->HasTangentsAndBitangents())
            {
                vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
                vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
            }

            if (mesh->HasTextureCoords(0))
                vertex.Texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            m_Vertices.push_back(vertex);
        }

        m_Indices.reserve(mesh->mNumFaces);

        for (size_t i = 0; i < m_Indices.capacity(); i++)
        {
            UME_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.");
            m_Indices.push_back({ mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] });
        }

        auto& vertexBuffer = VertexBuffer::Create(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
        auto& indexBuffer  = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * 3);
        vertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float3, "a_Tangent"},
            {ShaderDataType::Float3, "a_Binormal"},
            {ShaderDataType::Float2, "a_Texcoord"},
        });

        m_VertexArray = VertexArray::Create();
        m_VertexArray->AddVertexBuffer(vertexBuffer);
        m_VertexArray->SetIndexBuffer(indexBuffer);
    }

    Mesh::~Mesh()
    {
    }
}