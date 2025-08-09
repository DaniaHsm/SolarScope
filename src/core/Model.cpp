#include "Model.hpp"
#include "TextureLoader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <iostream>

using namespace glm;

void Model::Draw(GLuint shader) {
    for (const auto& mesh : meshes) {
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.texture);
        GLint texLocation = glGetUniformLocation(shader, "texture1");
        if (texLocation == -1) {
            std::cerr << "Warning: Uniform 'texture1' not found in shader" << std::endl;
        }
        glUniform1i(texLocation, 0);

        // Draw mesh
        glBindVertexArray(mesh.VAO);
        if (mesh.indices.empty()) {
            std::cerr << "Warning: Mesh has no indices" << std::endl;
        } else {
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);

        // Check for OpenGL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error in Draw: " << err << std::endl;
        }
    }
}

Model Model::loadFromFile(const char* path) {
    Model model;
    Assimp::Importer importer;
    std::cout << "Attempting to load model from: " << path << std::endl;

    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices |
        aiProcess_ValidateDataStructure | aiProcess_PreTransformVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return model;
    }

    std::cout << "Model loaded successfully. Number of meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "Number of materials: " << scene->mNumMaterials << std::endl;
    std::cout << "Number of children in root node: " << scene->mRootNode->mNumChildren << std::endl;

    // Process all nodes recursively starting from the root
    processNode(model, scene->mRootNode, scene);

    return model;
}

void Model::processNode(Model& model, aiNode* node, const aiScene* scene) {
    // Process all meshes in this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        if (!mesh) {
            std::cerr << "Warning: Null mesh encountered" << std::endl;
            continue;
        }

        std::cout << "Processing mesh with " << mesh->mNumVertices << " vertices" << std::endl;

        Mesh newMesh;

        // Load material/texture first
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            if (material) {
                aiString texturePath;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                    std::string fullPath = std::string("models/rubber_duck/textures/material_baseColor.jpeg");
                    std::cout << "Loading texture from: " << fullPath << std::endl;
                    newMesh.texture = TextureLoader::loadTexture(fullPath.c_str());
                    if (newMesh.texture == 0)
                    {
                        std::cerr << "Failed to load texture!" << std::endl;
                    }
                    else
                    {
                        std::cout << "Texture loaded successfully with ID: " << newMesh.texture << std::endl;
                    }
                }
            }
        }

        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            newMesh.vertices.push_back(vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));

            if (mesh->HasNormals()) {
                newMesh.normals.push_back(vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
            }

            if (mesh->mTextureCoords[0]) {
                newMesh.texCoords.push_back(vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
            } else {
                newMesh.texCoords.push_back(vec2(0.0f, 0.0f));
            }
        }

        // Process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                newMesh.indices.push_back(face.mIndices[j]);
            }
        }

        if (!newMesh.vertices.empty() && !newMesh.indices.empty()) {
            newMesh.setupMesh();
            model.meshes.push_back(newMesh);
            std::cout << "Added mesh with " << newMesh.vertices.size() << " vertices and " 
                     << newMesh.indices.size() << " indices" << std::endl;
        }
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(model, node->mChildren[i], scene);
    }
}
