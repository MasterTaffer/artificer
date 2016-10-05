#version 330 core

//Simple texture fragment shader

out vec4 color;
uniform sampler2D current;
in vec2 texCoord;

void main()
{
	color = texture(current,texCoord);
}
