#include "include/utils/TextureUtils.hpp"
#include "stb_image.h"

GLuint TextureUtils::loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    std::cout << "Loading texture from path: " << path << std::endl;

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        std::cout << "Texture loaded successfully: " << width << "x" << height 
                  << " channels: " << nrChannels << std::endl;

        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Check for OpenGL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error in loadTexture: " << err << std::endl;
        }

        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        stbi_image_free(data);
        return 0; // Return 0 to indicate failure
    }
    return textureID;
}
