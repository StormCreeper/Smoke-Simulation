#ifndef VOXEL_TEXTURE_HPP
#define VOXEL_TEXTURE_HPP

#include "gl_includes.hpp"
#include "shader.hpp"
#include "texture3D.hpp"
#include "compute_shader.hpp"

class Simulation {
   public:
	Texture3D densVelTexture{};
	Texture3D divergenceTexture{};
	Texture3D divFreeTexture{};
	Texture3D curlTexture{};

	ComputeShader volumeGenShader{};
	ComputeShader diffuseShader{};
	ComputeShader advectShader{};
	ComputeShader divShader{};
	ComputeShader solveDivShader{};
	ComputeShader nablaGShader{};
	ComputeShader curlShader{};
	ComputeShader confForceShader{};

	GLuint dimXZ{};
	GLuint dimY{};

	std::vector<float> data{};

	float m_conf = 3.0f;

   public:
	Simulation() {
		dimXZ = 128;
		dimY = 128;
	}
	~Simulation() {
	}

	int index(int x, int y, int z, int channel) {
		return 4 * (x + y * dimXZ + z * dimXZ * dimY) + channel;
	}

	void init_textures() {
		volumeGenShader.init("../../data/shaders/compute/volume_gen.glsl", dimXZ, dimY);
		diffuseShader.init("../../data/shaders/compute/diffuse.glsl", dimXZ, dimY);
		advectShader.init("../../data/shaders/compute/advect.glsl", dimXZ, dimY);
		divShader.init("../../data/shaders/compute/div.glsl", dimXZ, dimY);
		solveDivShader.init("../../data/shaders/compute/solve_div.glsl", dimXZ, dimY);
		nablaGShader.init("../../data/shaders/compute/nabla_g.glsl", dimXZ, dimY);
		curlShader.init("../../data/shaders/compute/curl.glsl", dimXZ, dimY);
		confForceShader.init("../../data/shaders/compute/conf_force.glsl", dimXZ, dimY);

		densVelTexture.init(dimXZ, dimY);

		volumeGenShader.use();

		volumeGenShader.run({}, {}, &densVelTexture);

		divergenceTexture.init(dimXZ, dimY);

		std::vector<float> zeroData(dimXZ * dimY * dimXZ * 4, 0.0f);
		divFreeTexture.init(dimXZ, dimY, zeroData);
		curlTexture.init(dimXZ, dimY, zeroData);
	}

	void simulationStep(glm::vec3 targetSize, glm::vec3 targetOffest, float dt) {
		diffuseShader.use();

		setUniform(diffuseShader.id(), "dt", dt);
		setUniform(diffuseShader.id(), "mu_density", 0.005f);
		setUniform(diffuseShader.id(), "mu_velocity", 0.001f);

		for (int i = 0; i < 5; i++) {
			diffuseShader.run({&densVelTexture}, {"u_inputImg"}, &densVelTexture);
		}

		advectShader.use();

		setUniform(advectShader.id(), "dt", dt);
		setUniform(advectShader.id(), "u_mask", glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));

		advectShader.run({&densVelTexture, &densVelTexture}, {"u_inputImg", "u_velocity"}, &densVelTexture);

		curlShader.use();
		curlShader.run({&densVelTexture}, {"u_inputImg"}, &curlTexture);

		confForceShader.use();
		setUniform(confForceShader.id(), "dt", dt);
		setUniform(confForceShader.id(), "confinement", m_conf);
		confForceShader.run({&densVelTexture, &curlTexture}, {"u_inputImg", "u_curl"}, &densVelTexture);

		divShader.use();
		divShader.run({&densVelTexture}, {"u_inputImg"}, &divergenceTexture);

		solveDivShader.use();
		for (int i = 0; i < 25; i++) {
			setUniform(solveDivShader.id(), "first_time", i == 0 ? 1 : 0);
			solveDivShader.run({&divFreeTexture, &divergenceTexture}, {"u_inputImg", "u_divergence"}, &divFreeTexture);
		}

		nablaGShader.use();
		nablaGShader.run({&divFreeTexture, &densVelTexture}, {"u_inputImg", "u_velocity"}, &densVelTexture);

		advectShader.use();

		setUniform(advectShader.id(), "dt", dt);
		setUniform(advectShader.id(), "u_mask", glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

		advectShader.run({&densVelTexture, &densVelTexture}, {"u_inputImg", "u_velocity"}, &densVelTexture);

		glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindTexture(GL_TEXTURE_3D, 0);

		glUseProgram(0);
	}

	GLuint getTextureID(int tex) {
		switch (tex) {
			case 1:
				return divergenceTexture.textureID;
			case 2:
				return divFreeTexture.textureID;
			case 3:
				return curlTexture.textureID;
			default:
				return densVelTexture.textureID;
		}
	}
};

#endif	// voxeltexture.hpp