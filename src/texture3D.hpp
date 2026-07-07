#ifndef TEXTURE3D_HPP
#define TEXTURE3D_HPP

#include "gl_includes.hpp"

class Texture3D {
   public:
	GLuint textureID{};

	void init(int dimXZ, int dimY, std::vector<float> data) {
		if (textureID) {
			destroy();
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_3D, textureID);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, dimXZ, dimY, dimXZ, 0, GL_RGBA, GL_FLOAT, data.data());

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_3D, 0);
	}

	void init(int dimXZ, int dimY) {
		if (textureID) {
			destroy();
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_3D, textureID);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, dimXZ, dimY, dimXZ, 0, GL_RGBA, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_3D, 0);
	}

	void bind() {
		glBindTexture(GL_TEXTURE_3D, textureID);
	}

	void destroy() {
		glDeleteTextures(1, &textureID);
	}

	~Texture3D() {
		if (textureID) {
			destroy();
		}
	}
};

#endif	// TEXTURE3D_HPP