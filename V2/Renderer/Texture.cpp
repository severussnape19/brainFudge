#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma message(">>> COMPILING MY stb_image.h <<<")


#include "Texture.hpp"
#include <iostream>


Texture::Texture(const std::string& path) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	std::cout << "Texture id = " << textureID << "\n";

	// sets texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	stbi_set_flip_vertically_on_load(true);

	int width, height, channels;
	std::cout << "Loading texture: " << path << "\n";

	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

	if (!data) {
		std::cerr << "FAILED: " << path << "\n";
		std::cerr << "Reason: " << stbi_failure_reason() << "\n";
	}
	else {
		std::cout << "SUCCESS: " << width << "x" << height << " channels=" << channels << "\n";
	}


	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA,
		width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

Texture::~Texture() {
	glDeleteTextures(1, &textureID);
}

void Texture::bind(unsigned int slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, textureID);
}