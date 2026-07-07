#define _USE_MATH_DEFINES

#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "object3d.hpp"
#include "framebuffer.hpp"
#include "simulation.hpp"
#include "cloudsmanager.hpp"
#include "scene.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "gl_includes.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

// Window parameters
GLFWwindow* g_window{};

// GPU objects
GLuint g_geometryShader{};	// A GPU program contains at least a vertex shader and a fragment shader
GLuint g_lightingShader{};	// A GPU program contains at least a vertex shader and a fragment shader

std::shared_ptr<FrameBuffer> g_framebuffer{};

Simulation g_simulation{};
CloudsManager g_cloudsManager{};

Scene g_scene{};

float g_fps = 0.0f;

float g_dt = 0.05f;
bool g_running = true;

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
	g_scene.m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
	glViewport(0, 0, (GLint)width, (GLint)height);	// Dimension of the rendering region in the window
}

bool shiftPressed = false;

// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if ((key == GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, true);

		if (key == GLFW_KEY_LEFT_SHIFT)
			shiftPressed = true;

		if (key == GLFW_KEY_R)
			g_simulation.init_textures();

		if (key == GLFW_KEY_SPACE)
			g_running = !g_running;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT_SHIFT) shiftPressed = false;
	}
}

float g_cameraDistance = 20.0f;
float g_cameraAngleX = 0.0f;

float g_yaw = 0.0f;
float g_pitch = 0.0f;

// Scroll for zooming
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	/*g_cameraDistance -= yoffset * 0.1f;
	g_cameraDistance = std::max(g_cameraDistance, 0.1f);

	g_cameraAngleX -= xoffset * 0.04f;*/

	if (shiftPressed) {
		g_cameraDistance -= yoffset * 0.1f;
		g_cameraDistance = std::max(g_cameraDistance, 0.1f);
	} else {
		g_yaw += xoffset * 0.04f;
		g_pitch += yoffset * 0.04f;
	}
}

void errorCallback(int error, const char* desc) {
	std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
	glfwSetErrorCallback(errorCallback);

	// Initialize GLFW, the library responsible for window management
	if (!glfwInit()) {
		std::cerr << "ERROR: Failed to init GLFW" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// Create the window
	g_window = glfwCreateWindow(
		1024, 768,
		"Interactive 3D Applications (OpenGL) - Volumetric clouds rendering", nullptr, nullptr);
	if (!g_window) {
		std::cerr << "ERROR: Failed to open window" << std::endl;
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent(g_window);
	glfwSetWindowSizeCallback(g_window, windowSizeCallback);
	glfwSetKeyCallback(g_window, keyCallback);
	glfwSetScrollCallback(g_window, scrollCallback);
}

void initImGui() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(g_window, true);  // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}

void initOpenGL() {
	// Load extensions for modern OpenGL
	if (!gladLoadGL()) {
		std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
		glfwTerminate();
		std::exit(EXIT_FAILURE);
	}

	glEnable(GL_DEBUG_OUTPUT);
	glCullFace(GL_BACK);	  // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable(GL_CULL_FACE);	  // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc(GL_LESS);	  // Specify the depth test for the z-buffer
	glEnable(GL_DEPTH_TEST);  // Enable the z-buffer test in the rasterization

	g_framebuffer = std::make_shared<FrameBuffer>(1024, 768);

	// Disable v-sync
	glfwSwapInterval(0);
}

void initGPUprogram() {
	g_geometryShader = glCreateProgram();  // Create a GPU program, i.e., two central shaders of the graphics pipeline
	loadShader(g_geometryShader, GL_VERTEX_SHADER, "../../data/shaders/geometryVertex.glsl");
	loadShader(g_geometryShader, GL_FRAGMENT_SHADER, "../../data/shaders/geometryFragment.glsl");
	glLinkProgram(g_geometryShader);  // The main GPU program is ready to be handle streams of polygons

	g_lightingShader = glCreateProgram();  // Create a GPU program, i.e., two central shaders of the graphics pipeline
	loadShader(g_lightingShader, GL_VERTEX_SHADER, "../../data/shaders/lightingVertex.glsl");
	loadShader(g_lightingShader, GL_FRAGMENT_SHADER, "../../data/shaders/lightingFragment.glsl");
	glLinkProgram(g_lightingShader);  // The main GPU program is ready to be handle streams of polygons
}

void init() {
	initGLFW();
	initOpenGL();

	int width, height;
	glfwGetWindowSize(g_window, &width, &height);

	g_scene.init(width, height);
	initGPUprogram();

	initImGui();

	g_simulation.init_textures();

	g_cloudsManager.setDefaults();
}

void clear() {
	glDeleteProgram(g_geometryShader);
	glDeleteProgram(g_lightingShader);

	glfwDestroyWindow(g_window);
	glfwTerminate();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

int Selecteditem = 0;

void renderPerfsUI() {
	ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("FPS: %.1f", g_fps);
	ImGui::Text("Frame time: %.3f ms", 1000.0f / g_fps);

	ImGui::Checkbox("Simulation running", &g_running);

	static const char* items[]{"Volume", "Divergence", "Div Solve", "Curl"};
	ImGui::Combo("Show texture", &Selecteditem, items, IM_ARRAYSIZE(items));

	ImGui::SliderFloat("Simulation dt", &g_dt, 0.0f, 1.0f);
	ImGui::SliderFloat("Confinement factor", &g_simulation.m_conf, 0.0f, 10.0f);

	ImGui::End();
}
void renderLightsUI() {
	ImGui::Begin("Lights", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	const char* items[] = {"Ambiant", "Point", "Directional"};

	for (int i = 0; i < g_scene.m_numLights; i++) {
		Light& light = g_scene.m_lights[i];
		if (ImGui::CollapsingHeader(std::string("Light " + std::to_string(i)).c_str())) {
			// ImGui::Text("Light %d", i);

			const char* comboLabel = items[light.type];

			if (ImGui::BeginCombo(("Type" + std::to_string(i)).c_str(), comboLabel)) {	// Combo box for type selection
				for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
					const bool is_selected = (comboLabel == items[n]);
					if (ImGui::Selectable(items[n], is_selected)) {
						light.type = n;
					}
					if (is_selected) {
						ImGui::SetItemDefaultFocus();  // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
					}
				}
				ImGui::EndCombo();
			}

			if (light.type != 0)
				ImGui::SliderFloat3(((light.type == 1 ? "Position" : "Direction") + std::to_string(i)).c_str(), &light.position.x, -10.0f, 10.0f);
			ImGui::ColorEdit3(("Color" + std::to_string(i)).c_str(), &light.color.x);
			ImGui::SliderFloat(("Intensity" + std::to_string(i)).c_str(), &light.intensity, 0.0f, light.type == 0 ? 1.0f : 10.0f);
		}
	}
	if (g_scene.m_numLights < MAX_LIGHTS) {
		if (ImGui::Button("Add light")) {
			g_scene.m_lights[g_scene.m_numLights++] = Light{
				1,
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(1.0, 1.0, 1.0),
				1.0f};
		}
		ImGui::SameLine();
	}
	if (g_scene.m_numLights > 0 && ImGui::Button("Remove light")) {
		g_scene.m_numLights--;
	}

	ImGui::End();
}
void renderUI() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Start drawing here

	renderPerfsUI();
	renderLightsUI();
	g_cloudsManager.renderUI();

	// End drawing here

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// The main rendering call
void render() {
	// Geometry pass
	glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer->m_Buffer);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				 // specify the background color, used any time the framebuffer is cleared
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	 // Erase the color and z buffers.
	glUseProgram(g_geometryShader);

	g_scene.geometryPass(g_geometryShader);

	// Post-process pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(g_lightingShader);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				 // specify the background color, used any time the framebuffer is cleared
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	 // Erase the color and z buffers.

	setUniform(g_lightingShader, "u_Position", 0);
	setUniform(g_lightingShader, "u_Normal", 1);
	setUniform(g_lightingShader, "u_Albedo", 2);

	const glm::mat4 viewMatrix = g_scene.m_camera.computeViewMatrix();
	const glm::mat4 projMatrix = g_scene.m_camera.computeProjectionMatrix();

	setUniform(g_lightingShader, "u_viewMat", viewMatrix);
	setUniform(g_lightingShader, "u_projMat", projMatrix);
	setUniform(g_lightingShader, "u_invViewMat", glm::inverse(viewMatrix));
	setUniform(g_lightingShader, "u_invProjMat", glm::inverse(projMatrix));

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, g_simulation.getTextureID(Selecteditem));

	setUniform(g_lightingShader, "u_voxelTexture", 3);

	g_scene.setUniforms(g_lightingShader);
	g_cloudsManager.setUniforms(g_lightingShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_framebuffer->m_position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_framebuffer->m_normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_framebuffer->m_albedo);

	glBindVertexArray(g_framebuffer->m_quad->m_vao);
	glDrawElements(GL_TRIANGLES, g_framebuffer->m_quad->m_numIndices, GL_UNSIGNED_INT, 0);

	renderUI();
}

int frameCount = 0;
// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {
	// Update the FPS computation
	static int frameCount = 0;
	static float lastTime = glfwGetTime();
	frameCount++;

	if (currentTimeInSec - lastTime >= 1.0) {
		g_fps = static_cast<float>(frameCount) / (currentTimeInSec - lastTime);

		frameCount = 0;
		lastTime = currentTimeInSec;
	}

	// Update the camera position

	glm::vec3 targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	g_scene.m_camera.setTarget(targetPosition);

	// glm::vec3 cameraOffset = glm::normalize(glm::vec3(cos(g_cameraAngleX), 0.3f, sin(g_cameraAngleX))) * (1.1f + g_cameraDistance);
	glm::vec4 cameraOffset(0, 0, 1, 0);

	glm::mat4 rot1 = glm::rotate(glm::mat4(1), g_yaw, glm::vec3(0, 1, 0));
	glm::mat4 rot2 = glm::rotate(glm::mat4(1), g_pitch, glm::vec3(1, 0, 0));

	cameraOffset = g_cameraDistance * rot1 * rot2 * cameraOffset;

	g_scene.m_camera.setPosition(targetPosition + glm::vec3(cameraOffset));

	if (g_running) {
		g_simulation.simulationStep(g_cloudsManager.m_generationParams.domainSize, g_cloudsManager.m_generationParams.domainCenter, g_dt);
	}

	frameCount++;
}

int main(int argc, char** argv) {
	init();
	while (!glfwWindowShouldClose(g_window)) {
		update(static_cast<float>(glfwGetTime()));
		render();
		glfwSwapBuffers(g_window);
		glfwPollEvents();
	}
	clear();
	return EXIT_SUCCESS;
}
