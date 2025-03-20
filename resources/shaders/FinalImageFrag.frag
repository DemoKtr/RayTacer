#version 450
layout(set=0,binding=0) uniform sampler2D colorMap;

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 inUV;


void main() {

	
	vec3 color = vec3 (0.0f,0.0f,0.0f);
	color = texture(colorMap,inUV.xy).rgb;
	outColor = vec4(color,1.0f);
}