#version 330 core
layout(location = 0) in vec2 VertexPosition;

//Default blit vertex shader

out vec2 texCoord;

void main()
{
	texCoord = (VertexPosition + 1) / 2;
	
	//Reverse the texcoord y, because the display is flipped
	texCoord.y = 1 - texCoord.y;
	gl_Position = vec4(VertexPosition,0,1);
}

