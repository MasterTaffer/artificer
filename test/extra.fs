#version 330 core
//See main.fs for some descriptions

out vec4 color;
in vec2 texCoord;

uniform sampler2D current;
uniform sampler2D pass_0;

uniform int frame;


//Uniforms defined in the configuration file
//psychedelic shadow offset
uniform float posy;
uniform float posx;


void main()
{
	//start with the input
	vec4 cmd = texture(current,texCoord);
	float st = cmd.x+cmd.y+cmd.z; 
	color = vec4(smoothstep(0.5,1.5,st));
	
	
	//get the slightly offset pass_0
	
	vec2 scm = texCoord-0.5;
	scm *= 0.85;
	scm += 0.5;
	vec4 tmdr = texture(pass_0,scm+vec2(posx,posy))*0.5;
	

	//ADD
	color += tmdr;
	
	//no negative colors allowed
	color = abs(color);
	color.a = 1.0;
}
