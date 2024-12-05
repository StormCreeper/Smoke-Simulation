#version 460 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout (rgba32f, binding = 0) uniform image3D img_output;

void main() {
	imageStore(img_output, coords, vec4(0.0));
}