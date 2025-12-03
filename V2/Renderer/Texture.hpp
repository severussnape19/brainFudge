#pragma once

#include <glad/glad.h>
#include <string>

class Texture {
public:
	Texture(const std::string& path);
	~Texture();

	void bind(unsigned int slot) const;
private:
	GLuint textureID = 0;
};
