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

    vec3 LightPos = vec3(-1.0f,1.0f,-1.0f);
	gl_Position = mProjection*mView*mWorld*vec4(inPosition, 1.0);
    float LightCoef = dot(normalize(LightPos), inNormal) / (length(LightPos) * length(inNormal));
    vec3 outputColor=vec3(LightCoef,LightCoef,LightCoef);
	theColor = outputColor;
}