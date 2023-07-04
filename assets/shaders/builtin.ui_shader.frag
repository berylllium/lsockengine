#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform object_uniform_object
{
	vec4 diffuse_color;
} object_uniform;

layout(set = 1, binding = 1) uniform sampler2D diffuse_sampler;

layout(location = 0) in struct dto
{
	vec2 tex_coord;
} in_dto;

layout(location = 0) out vec4 out_colour;

void main()
{
	out_colour = object_uniform.diffuse_color * texture(diffuse_sampler, in_dto.tex_coord);
}
