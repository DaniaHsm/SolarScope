#pragma once
#include <GL/glew.h>
#include <string>
#include <iostream>

class TextureUtils {
public:
    static GLuint loadTexture(const char* path);
};
