#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform SceneData{
	vec4 fogColor;
	vec4 fogDistances;
	vec4 ambientColor;
	vec4 sunlightDirection;
	vec4 sunlightColor;
} sceneData;

void main(){
	outColor = vec4(fragColor + sceneData.ambientColor.xyz, 1.0);
}