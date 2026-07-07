#version 460 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout (rgba32f, binding = 0) uniform image3D img_output;

uniform sampler3D u_inputImg;
uniform sampler3D u_curl;

uniform float dt;

uniform float confinement;

void main() {
    ivec3 dims = imageSize(img_output);
    ivec3 coords = ivec3(gl_GlobalInvocationID.xyz);

    if(coords.x == 0 || coords.y == 0 || coords.z == 0 || coords.x == dims.x - 1 || coords.y == dims.y - 1 || coords.z == dims.z - 1) {
        return;
    }

    vec4 base_val = texelFetch(u_inputImg, coords, 0);

    vec3 curl = texelFetch(u_curl, coords, 0).gba;

    vec3 grad_curl = vec3(
        texelFetch(u_curl, coords + ivec3(1, 0, 0), 0).r - texelFetch(u_curl, coords - ivec3(1, 0, 0), 0).r,
        texelFetch(u_curl, coords + ivec3(0, 1, 0), 0).r - texelFetch(u_curl, coords - ivec3(0, 1, 0), 0).r,
        texelFetch(u_curl, coords + ivec3(0, 0, 1), 0).r - texelFetch(u_curl, coords - ivec3(0, 0, 1), 0).r
    ) * 0.5;

    grad_curl = grad_curl / (length(grad_curl) + 0.00001f);

    vec3 f_conf = dt * confinement * cross(grad_curl, curl);

	imageStore(img_output, coords, base_val + vec4(0.0, f_conf.x, f_conf.y, f_conf.z));
}