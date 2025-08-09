#pragma once
#include <GL/glew.h>
#include <assimp/scene.h>
#include <vector>
#include "Mesh.hpp"

struct Model {
    std::vector<Mesh> meshes;

    void Draw(GLuint shader);
    static Model loadFromFile(const char* path);

private:
    static void processNode(Model& model, aiNode* node, const aiScene* scene);
};
