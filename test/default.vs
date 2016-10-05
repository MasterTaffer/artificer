#version 330 core
layout(location = 0) in vec2 VertexPosition;

//Quite straightforward vertex shader

out vec2 texCoord;

void main()
{
	texCoord = (VertexPosition + 1) / 2;
	texCoord.y = texCoord.y;
	gl_Position = vec4(VertexPosition,0,1);
}

