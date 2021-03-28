#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vColor;
layout (location = 2) in vec3 vNormal;

layout (location = 0) out vec3 fragColor;

//push constants block
layout(push_constant) uniform constants
{
	vec4 data;
	mat4 render_matrix;
} PushConstants;

void main() 
{
	gl_Position = PushConstants.render_matrix * vec4(vPosition, 1.0f);
	fragColor = vColor;
}