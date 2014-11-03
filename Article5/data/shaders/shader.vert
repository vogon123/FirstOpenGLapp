#version 330

uniform mat4 mWorld;
uniform mat4 mView;
uniform mat4 mProjection;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

smooth out vec3 theColor;

void main()
{
	gl_Position = mProjection * mView * mWorld * vec4(inPosition, 1.0);
    vec3 vLightPos = vec3(-2.0, 2.0, 0.0);
    vec3 vLightColor = vec3(1.0, 1.0, 1.0);
    vec3 outputColor=vec3(0.5f,0.5f,0.5f);
	outputColor+=clamp(dot(inNormal,normalize(vLightPos-inPosition)), 0, 1)*vLightColor;
    theColor = outputColor;
}