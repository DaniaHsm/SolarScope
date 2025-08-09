#pragma once
#include <GL/glew.h>
#include <string>
#include <iostream>

class TextureLoader {
public:
    static GLuint loadTexture(const char* path);
};
