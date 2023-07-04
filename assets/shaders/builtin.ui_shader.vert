#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texcoord;

layout(set = 0, binding = 0) uniform global_uniform_object {
	mat4 projection;
	mat4 view;
} global_ubo;

layout(push_constant) uniform u_push_constants
{
	mat4 model;
} push_constants;

layout(location = 0) out struct dto
{
	vec2 tex_coord;
} out_dto;

void main()
{
	// Flip y-coord. This puts [0, 0] in the top left, instead of the bottom left.
	out_dto.tex_coord = vec2(in_texcoord.x, 1.0 - in_texcoord.y);

	gl_Position = global_ubo.projection * global_ubo.view * push_constants.model * vec4(in_position, 0.0, 1.0);
}
