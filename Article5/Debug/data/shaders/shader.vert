#version 330

uniform mat4 mWorld;
uniform mat4 mView;
uniform mat4 mProjection;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTex;

smooth out vec3 theColor;

void main()
{
	gl_Position = mProjection*mView*mWorld*vec4(inPosition, 1.0);
	vec3 pos= vec3(gl_Position)*0.5f;
	vec3 ligt=vec3(0.25f,0.0f,1.0f);
	vec3 color=dot(ligt,pos) * (vec3(0.75f+inTex.x,0.75f+inTex.y+inTex.x,0.75f+inTex.y)) *0.3f;
	theColor = color;
}